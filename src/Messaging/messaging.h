/*
 * messaging.h
 *
 *  Created on: 21 сент. 2014 г.
 *      Author: Prophet
 */

#ifndef MESSAGING_H_
#define MESSAGING_H_

#include <Arduino.h>

typedef struct{
  byte SensorID;        // идентификатор датчика
  byte CommandTo;       // команда модулю номер ...
  byte Command;         // команда
                            // 0 - нет команды или ответ
                            // 1 - получить значение
                            // 2 - установить значение
  byte ParamID;         // идентификатор параметра
  float ParamValue;    // значение параметра
  boolean Status;      // статус 0 - ошибка, 1 - ок
  char Comment[16];    // комментарий
} Message;

typedef struct{
  float Value;         // значение
  boolean Status;      // статус - 0-ошибка (false), 1-ок (true)
  char Note[16];       // комментарий
}  Parameter;

#endif /* MESSAGING_H_ */
