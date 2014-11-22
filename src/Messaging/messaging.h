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
	uint8_t iDeviceID;		// id устройства
	char cDescription[];	// описание устройства
	uint8_t localAdress[6];	// адрес устройства
} DeviceDescriptor;

typedef struct {
	uint8_t iNodeID;			// id узла
	uint8_t iNodeType;			// тип узла
	char cDescription[];		// описание узла
	uint8_t remoteAddress[6];	// адрес радио
} NodeDescriptor;

typedef struct {
	uint8_t iType;		// тип значения
	float 	fValue;		// значение
	boolean bStatus;	// статус - 0-ошибка (false), 1-ок (true)
	char 	cNote[];	// комментарий
} NodeValue;

typedef struct {
	uint8_t sourceAddress[5];
	uint8_t iSequenceSize;
	uint8_t iSequenceNumber;
	byte* payload;
} Packet;

#endif /* MESSAGING_H_ */
