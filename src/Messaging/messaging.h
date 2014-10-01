/*
 * messaging.h
 *
 *  Created on: 21 сент. 2014 г.
 *      Author: Prophet
 */

#ifndef MESSAGING_H_
#define MESSAGING_H_

#include <Arduino.h>

typedef struct {
	uint8_t iDeviceID;
	char cDescription[32];
} DeviceDescriptor;

typedef struct {
	uint8_t iNodeID;
	uint8_t iNodeType;
	char cDescription[32];
} NodeDescriptor;

typedef struct {
  float fValue;         // значение
  boolean bStatus;      // статус - 0-ошибка (false), 1-ок (true)
  char cNote[16];       // комментарий
} NodeValue;

typedef struct {
	uint8_t iPacketType;
	uint8_t iSenderID;
	uint8_t iDestinationID;
	uint8_t iPayloadSize;
	byte* payload;
	uint16_t wChecksum;
} Packet;

#endif /* MESSAGING_H_ */
