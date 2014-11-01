/*
 * main.cpp
 *
 *  Created on: 20 сент. 2014 г.
 *      Author: Prophet
 */

#include <Arduino.h>

#include "SPI\SPI.h"
#include "RF24\nRF24L01.h"
#include "RF24\RF24.h"
#include "printf.h"
#include "DHT\DHT.h"
#include "Messaging\Messaging.h"
#include "ChainableLED\ChainableLED.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(8, 7);

// DHT Sensor
#define DHTPIN A0     // what pin we're connected to// Uncomment whatever type you're using!//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)//#define DHTTYPE DHT21   // DHT 21 (AM2301)DHT dht(DHTPIN, DHTTYPE);

ChainableLED tempLED(3, 5, 1);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] =
{ 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum
{
	role_ping_out = 1, role_pong_back
} role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] =
{ "invalid", "Ping out", "Pong back" };

// The role of the current running sketch
role_e role = role_pong_back;

float humidity = 0;
float temperature = 0;

void setup(void)
{
	// Button
	pinMode(4, INPUT);
	// enable pull-up resistor
	digitalWrite(4, HIGH);

	// LED
	pinMode(9, OUTPUT);

	// Print preamble
	Serial.begin(57600);

	// Setup and configure rf radio
	radio.begin();

	// optionally, increase the delay between retries & # of retries
	radio.setRetries(15, 15);

	// Open pipes to other nodes for communication

	// This simple sketch opens two pipes for these two nodes to communicate
	// back and forth.
	// Open 'our' pipe for writing
	// Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

	if (role == role_ping_out)
	{
		radio.openWritingPipe(pipes[0]);
		radio.openReadingPipe(1, pipes[1]);
	}
	else
	{
		radio.openWritingPipe(pipes[1]);
		radio.openReadingPipe(1, pipes[0]);
	}

	// Start listening
	radio.startListening();

	// init DHT
	dht.begin();

	tempLED.setColorRGB(1, 255, 0, 0);
}

void DoBlink(byte bPin, int iCount, int iDelay = 500)
{
	for (int i = 0; i < iCount + 1; i++)
	{
		digitalWrite(bPin, HIGH);
		delay(iDelay);
		digitalWrite(bPin, LOW);
		delay(iDelay / 2);
	}
}

void rolePingOutExecute(void)
{
	// First, stop listening so we can talk.
	radio.stopListening();

	// Take the time, and send it.  This will block until complete
	unsigned long time = millis();
	bool ok = radio.write(&time, sizeof(unsigned long));

	if (ok)
	{
		;
	}

	// Now, continue listening
	radio.startListening();

	// Wait here until we get a response, or timeout (500ms)
	unsigned long started_waiting_at = millis();
	bool timeout = false;
	while (!radio.available() && !timeout)
		if (millis() - started_waiting_at > 500)
			timeout = true;

	// Describe the results
	if (timeout)
	{
		// Timed out, blink error
		DoBlink(9, 5, 500);
	}
	else
	{
		// Grab the response, compare, and send to debugging spew
		unsigned long got_time;
		radio.read(&got_time, sizeof(unsigned long));

//		printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
	}

	// Try again 1s later
	delay(1000);
}

void rolePongBackExecute(void)
{
	// if there is data ready
	if (radio.available())
	{
		// Dump the payloads until we've gotten everything
		unsigned long got_time;
		bool done = false;
		while (!done)
		{
			// Fetch the payload, and see if this was the last one.
			done = radio.read(&got_time, sizeof(unsigned long));
			// Delay just a little bit to let the other unit make the transition to receiver
			delay(20);
		}

		// First, stop listening so we can talk
		radio.stopListening();

		// Send the final one back.
		radio.write(&got_time, sizeof(unsigned long));
		DoBlink(9, 1, 200);

		// Now, resume listening so we catch the next packets.
		radio.startListening();
	}
}

void loop(void)
{
	switch (role)
	{
	case role_ping_out:
		// Ping out role.  Repeatedly send the current time
		rolePingOutExecute();
		break;
	case role_pong_back:
		// Pong back role.  Receive each packet, dump it out, and send it back
		rolePongBackExecute();
		break;
	}

	//
	// Change roles
	//

	bool b = digitalRead(4); // Using a digital unlatched button for it without jitter correction

	if ((b == LOW) && role == role_pong_back)
	{
		// Become the primary transmitter (ping out)
		role = role_ping_out;
		radio.openWritingPipe(pipes[0]);
		radio.openReadingPipe(1, pipes[1]);
		DoBlink(9, 3, 200); // Blink transmit mode
		delay(2000);
	}
	else if ((b == LOW) && role == role_ping_out)
	{
		// Become the primary receiver (pong back)
		role = role_pong_back;
		radio.openWritingPipe(pipes[1]);
		radio.openReadingPipe(1, pipes[0]);
		DoBlink(9, 2, 200); // blink recieve mode
		delay(2000);
	}

	float temp = dht.readTemperature();
	if (temp > 23)
		tempLED.setColorRGB(1, 100-(temp-23), 150+(temp-23),0);
	else
		tempLED.setColorRGB(1, 150+(23-temp), 100-(23-temp),0);
}

int main(void)
{
	init();
	setup();

	for (;;)
	{
		loop();
	}

	return 0;
}

