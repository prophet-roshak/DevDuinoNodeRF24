/*
 * messaging.cpp
 *
 *  Created on: 21 сент. 2014 г.
 *      Author: Prophet
 */

#include "messaging.h"


uint16_t WordCrc16(uint8_t *Data, uint16_t size)
{
	uint16_t w;
	uint8_t shift_cnt, f;
	uint8_t *ptrByte;
	uint16_t byte_cnt = size;
	ptrByte = Data;
	w = (uint16_t) 0xffff;
	for (; byte_cnt > 0; byte_cnt--)
	{
		w = (uint16_t) (w ^ (uint16_t) (*ptrByte++));
		for (shift_cnt = 0; shift_cnt < 8; shift_cnt++)
		{
			f = (uint8_t) ((w) & (0x1));
			w >>= 1;
			if ((f) == 1)
				w = (uint16_t) ((w) ^ 0xa001);
		}
	}
	return w;
}


