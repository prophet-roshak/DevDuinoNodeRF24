/*
 * core.hpp
 *
 *  Created on: 02 нояб. 2014 г.
 *      Author: Prophet
 */

#ifndef CORE_HPP_
#define CORE_HPP_

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Chainable LED
#include "ChainableLED\ChainableLED.h"

// RF24L01 Radio Lib
#include "SPI\SPI.h"
#include "RF24\nRF24L01.h"
#include "RF24\RF24.h"

// Humidity and Temp sensor
#include "DHT\DHT.h"

// Messaging subsystem
#include "Messaging\Messaging.h"

#endif /* CORE_HPP_ */
