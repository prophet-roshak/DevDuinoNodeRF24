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

// DHT Sensor
DHT dht(A0, DHT22); // Set up DHT at pin A0, type DHT22 - AM2302

// RGB Led
ChainableLED tempLED(3, 5, 1); // Setup 1 led at data pin 3, clk pin 5

/*
 * Topology
 */

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] =
{ 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

/*
 * State Variables
 */

volatile boolean wdt_trigger = 0;

#define SLEEP_WDT_TIMEOUT WDTO_8S // Defines watchdog timer divider, so timeout occurs every 8 sec.
#define SLEEP_TIMEOUT 4 // Defines sleep timeout before starting radio op = 4 * 8s = 32s

/*
 * ISR Routines
 */

// Global ISR for wakeup
ISR(WDT_vect) {
	wdt_trigger = 1;  // set global volatile variable
}

void globalInit(void) {
	// Watchdog initialization
	wdt_disable();
	wdt_reset();
	wdt_enable(SLEEP_WDT_TIMEOUT);   //пробуждение каждые 8 сек

	// Button, 	enable pull-up resistor
	pinMode(4, INPUT);
	digitalWrite(4, HIGH);

	// LED
	pinMode(9, OUTPUT);

	radio.begin();				// Setup and configure rf radio
	radio.setRetries(15, 15);	// optionally, increase the delay between retries & # of retries
	//radio.setChannel(76);

	// This node will only send data away, and get response packets if available
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);

	radio.stopListening();

	// init DHT
	dht.begin();

	tempLED.setColorRGB(0, 255, 0, 0);

	delay(200); // Successfull init delay
}

int main(void)
{
	init(); 		// Arduino init

	globalInit(); 	// Init hardware

	uint8_t timeout = SLEEP_TIMEOUT;
	for (;;)
	{
		// Reset watchdog timer, enable WDT interrupts
		wdt_interrupt_mode();

		// If watchdog was triggered lower timeout counter
		if (wdt_trigger)
		{
			timeout--;
			wdt_trigger = 0;
		}

		// If sleep timeout occured - execute actual job
		if (timeout <= 0)
		{
			ledBlink(9, 2, 200);

			// disable radio before sleep
			radio.powerDown();
			// reset sleep timeout
			timeout = SLEEP_TIMEOUT;
		}

		// Enter sleep mode
		system_sleep();
	}

	return 0;
}
