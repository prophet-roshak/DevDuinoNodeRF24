/*
 * utils.cxx
 *
 *  Created on: 04 нояб. 2014 г.
 *      Author: Prophet
 */

#include "core.hpp"

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

//режим сна для МК
void system_sleep() {
  delay(2);                            // Wait for serial traffic
  _SFR_BYTE(ADCSRA) &= ~_BV(ADEN);     // Switch ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();                        // System sleeps here
  sleep_disable();
  _SFR_BYTE(ADCSRA) |= _BV(ADEN);      // Switch ADC on
}

void wdt_interrupt_mode() {
  wdt_reset();
  WDTCSR |= _BV(WDIE); // Restore WDT interrupt mode
}
