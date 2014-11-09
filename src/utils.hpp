/*
 * utils.hpp
 *
 *  Created on: 04 нояб. 2014 г.
 *      Author: Prophet
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

void ledBlink(byte bPin, int iAmount, int iDelay = 500);

void system_sleep();

void wdt_interrupt_mode();

#endif /* UTILS_HPP_ */
