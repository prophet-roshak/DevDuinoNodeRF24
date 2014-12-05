/*
 * core.cxx
 *
 *  Created on: 04 нояб. 2014 г.
 *      Author: Prophet
 */

#include <Arduino.h>

// RF24L01 Radio Lib
#include <SPI\SPI.h>
#include "RF24\nRF24L01.h"
#include "RF24\RF24.h"
#include "RF24\printf.h"

// Device mode definition
enum HARV_MODE {HARV_RX, HARV_TX};

/*
 * Hardware setup
 */
RF24 radio(8, 7); // Set up nRF24L01 radio on SPI bus plus pins 9 & 10

/*
 * State Variables
 */

HARV_MODE mode = HARV_RX;

/*
 * Topology
 */

const uint8_t pipes[][6] = { {0xF0, 0xF0, 0xF0, 0xF0, 0xE1}, {0xF0, 0xF0, 0xF0, 0xF0, 0xD2} };

/*
 * ISR Routines
 */

/*
 * Main code
 */

void
globalInit(void) {

	// Button, 	enable pull-up resistor
	pinMode(4, INPUT);
	digitalWrite(4, HIGH);

	// Enable pull-up on interrupt pin
	digitalWrite(2, HIGH);

	// LED
	pinMode(9, OUTPUT);

	radio.begin();				// Setup and configure rf radio
	radio.stopListening();
	radio.setPALevel(RF24_PA_HIGH);

	delay(200); // Successfull init delay
}

void ledBlink(byte bPin, int iAmount, int iDelay = 500)
{
	for (int i = 0; i < iAmount; i++)
	{
		digitalWrite(bPin, HIGH);
		delay(iDelay);
		digitalWrite(bPin, LOW);
		delay(iDelay / 2);
	}
}

void pingPongInit(HARV_MODE mode)
{
	//radio.enableDynamicAck(); // [! if you uncomment this line, my devices will not clean TX_FIFO, like AUTOACK is disabled]
	radio.enableDynamicPayloads();

	if (mode == HARV_TX)
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
}

void rolePingOutExecute(void)
{
	ledBlink(9, 1, 200);

	// First, stop listening so we can talk.
	radio.stopListening();

	// Take the time, and send it.  This will block until complete
	unsigned long time = millis();
	bool ok = radio.writeFast(&time, sizeof(unsigned long));
	if (ok)
		ok = radio.txStandBy(30);

	// Now, continue listening
	radio.startListening();

	if (ok)
	{
		ledBlink(9, 1, 200);
	}

	// Wait here until we get a response, or timeout (500ms)
	unsigned long started_waiting_at = millis();
	bool timeout = false;
	while (!radio.available() && !timeout)
		if (millis() - started_waiting_at > 500)
			timeout = true;

	// Describe the results
	if (!timeout)
	{
		// Grab the response, compare, and send to debugging spew
		unsigned long got_time;
		radio.read(&got_time, sizeof(unsigned long));

		ledBlink(9, 1, 200);
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
			radio.read(&got_time, sizeof(unsigned long));
			if (!radio.available())
				done = true;
		}

		// Delay just a little bit to let the other unit make the transition to receiver
		delay(60);

		// First, stop listening so we can talk
		radio.stopListening();

		// Send the final one back.
		bool ok = radio.writeFast(&got_time, sizeof(unsigned long));
		if (ok)
			ok = radio.txStandBy(30);

		// Now, resume listening so we catch the next packets.
		radio.startListening();

		if (ok)
			ledBlink(9, 1, 200);
	}
}

int main(void)
{
	init(); 	// Arduino init
	globalInit(); 	// Init hardware

	pingPongInit(mode); // RF24 settings and pipes init

	for (;;)
	{
		if (mode == HARV_TX)
			rolePingOutExecute();
		else if (mode == HARV_RX)
			rolePongBackExecute();
	}

	return 0;
}
