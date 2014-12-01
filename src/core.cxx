/*
 * core.cxx
 *
 *  Created on: 04 нояб. 2014 г.
 *      Author: Prophet
 */

#include "core.hpp"
#include "utils.hpp"
#include "sensors.hpp"

#include "printf.h"

/*
 * Hardware setup
 */
RF24 radio(8, 7); // Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24Network network(radio); // set up RF24 Network superstructure

// DHT Sensor
DHT dht(A0, DHT22); // Set up DHT at pin A0, type DHT22 - AM2302

// RGB Led
ChainableLED tempLED(3, 5, 1); // Setup 1 led at data pin 3, clk pin 5

struct SensorData {
	float temp;
	float humidity;
	long vcc;
};

/*
 * Topology
 */

#include "sensornet.hpp"
uint16_t this_node;                                  // Our node address

/*
 * State Variables
 */

//volatile boolean wdt_trigger = 0;

#define SLEEP_WDT_TIMEOUT WDTO_8S // Defines watchdog timer divider, so timeout occurs every 8 sec.
#define SLEEP_TIMEOUT 4 // Defines sleep timeout before starting radio op = 4 * 8s = 32s

const short max_active_nodes = 10;                    // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

unsigned long awakeTime = 500;
unsigned long sleepTime = 0;

volatile uint8_t sendErrors;
volatile bool jobExecuted;

// Prototypes for functions to send & handle messages
bool send_D(uint16_t to);
void handle_D(RF24NetworkHeader& header);

bool send_T(uint16_t to);
void handle_T(RF24NetworkHeader& header);

bool send_N(uint16_t to);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);
// End prototypes declaration

//void mainjob();

/*
 * ISR Routines
 */

// Global ISR for wakeup
/*
ISR(WDT_vect) {
	wdt_trigger = 1;  // set global volatile variable
}
*/

void globalInit(void) {
	// Watchdog initialization
/*	wdt_disable();
	wdt_reset();
	wdt_enable(SLEEP_WDT_TIMEOUT);   //пробуждение каждые 8 сек
*/

	// Button, 	enable pull-up resistor
	pinMode(4, INPUT);
	digitalWrite(4, HIGH);

	// LED
	pinMode(9, OUTPUT);

	radio.begin();				// Setup and configure rf radio
	radio.setPALevel(RF24_PA_HIGH);
	//radio.setRetries(15, 15);	// optionally, increase the delay between retries & # of retries
	//radio.setChannel(76);

	// This node will only send data away, and get response packets if available
/*	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);

	radio.stopListening();*/


	// Network startup
	this_node = node_address_set[NODE_ADDRESS];
	network.begin(/*channel*/ 100, /*node address*/ this_node );

	/******************************** This is the configuration for sleep mode ***********************/
	network.setup_watchdog(SLEEP_WDT_TIMEOUT); // 8sec

	// init DHT
	dht.begin();

	tempLED.setColorRGB(0, 255, 0, 0);

	sendErrors = 0;
	jobExecuted = false;

	delay(200); // Successfull init delay
}

int main(void)
{
	init(); 		// Arduino init

	globalInit(); 	// Init hardware

	for (;;)
	{
		network.update();

		// do network part
		while (network.available())
		{
			RF24NetworkHeader header; // If network available, take a look at it
			network.peek(header);

			switch (header.type) { // Dispatch the message to the correct handler.

				case 'N':
					handle_N(header);
					break;

					/************* SLEEP MODE *********/
					// Note: A 'sleep' header has been defined, and should only need to be ignored if a node is routing traffic to itself
					// The header is defined as:  RF24NetworkHeader sleepHeader(/*to node*/ 00, /*type*/ 'S' /*Sleep*/);
				case 'S': /*This is a sleep payload, do nothing*/
					break;

				default:
					printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"), header.type);
					network.read(header, 0, 0);
					break;
			};
		}


		// MAIN JOB BEGINS
		if (!jobExecuted)
		{
			ledBlink(9, 1, 1000);

			SensorData data;

			data.temp = dht.readTemperature();
			data.humidity = dht.readHumidity();
			data.vcc = readVcc();

			RF24NetworkHeader header(/*to node*/ 00, /*type*/ 'D' /*Data*/);

			if (network.write(header,&data,sizeof(SensorData)))
			{
				ledBlink(9, 1, 200);
				sendErrors = 0;
			}
			else
			{
				ledBlink(9, 3, 200);
				sendErrors++;
			}
			jobExecuted = true;
		}
		// MAIN JOB ENDS

		/***************************** CALLING THE NEW SLEEP FUNCTION ************************/

		if (millis() - sleepTime > awakeTime && NODE_ADDRESS) { // Want to make sure the Arduino stays awake for a little while when data comes in. Do NOT sleep if master node.
			sleepTime = millis();                      // Reset the timer value
			radio.stopListening(); // Switch to PTX mode. Payloads will be seen as ACK payloads, and the radio will wake up

			if (sendErrors > 3) // this will overflow in 252 send retries, starting 3 short period sends...
				network.sleepNode(SLEEP_TIMEOUT * 30, 2); // Extended sleep the node for 4 * 30 cycles of 8 second intervals aprox 16 minutes
			else
				network.sleepNode(SLEEP_TIMEOUT, 2); // Sleep the node for 4 cycles of 8 second intervals aprox 32 sec
			jobExecuted = false;
		}

		//Examples:
		// network.sleepNode( cycles, interrupt-pin );
		// network.sleepNode(0,0);         // The WDT is configured in this example to sleep in cycles of 1 second. This will sleep 1 second, or until a payload is received
		// network.sleepNode(1,255);       // Sleep this node for 1 second. Do not wake up until then, even if a payload is received ( no interrupt ) Payloads will be lost.

		/****  end sleep section ***/

	}

	return 0;
}

bool send_D(uint16_t to)
{
	SensorData data;

	data.temp = dht.readTemperature();
	data.humidity = dht.readHumidity();
	data.vcc = readVcc();

	RF24NetworkHeader header(/*to node*/ to, /*type*/ 'D' /*Data*/);
	return network.write(header,&data,sizeof(SensorData));
}


/**
 * Send a 'T' message, the current time
 */
bool send_T(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);

  // The 'T' message that we send is just a ulong, containing the time
  unsigned long message = millis();
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"),millis(),message,to);
  return network.write(header,&message,sizeof(unsigned long));
}

/**
 * Send an 'N' message, the active node list
 */
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);

  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"),millis(),to);
  return network.write(header,active_nodes,sizeof(active_nodes));
}

/**
 * Handle a 'T' message
 * Add the node to the list of active nodes
 */
void handle_T(RF24NetworkHeader& header){

  unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  network.read(header,&message,sizeof(unsigned long));
  printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"),millis(),message,header.from_node);


  if ( header.from_node != this_node || header.from_node > 00 )                                // If this message is from ourselves or the base, don't bother adding it to the active nodes.
    add_node(header.from_node);
}

/**
 * Handle an 'N' message, the active node list
 */
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  network.read(header,&incoming_nodes,sizeof(incoming_nodes));
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"),millis(),header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
 * Add a particular node to the current list of active nodes
 */
void add_node(uint16_t node){

  short i = num_active_nodes;                                    // Do we already know about this node?
  while (i--)  {
    if ( active_nodes[i] == node )
        break;
  }

  if ( i == -1 && num_active_nodes < max_active_nodes ){         // If not, add it to the table
      active_nodes[num_active_nodes++] = node;
      printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"),millis(),node);
  }
}
