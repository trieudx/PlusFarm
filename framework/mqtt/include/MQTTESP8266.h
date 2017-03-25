/**
  ******************************************************************************
  * @file    MQTTESP8266.h
  * @author  Baoshi <mail(at)ba0sh1(dot)com>
  * @version 0.1
  * @date    Sep 9, 2015
  * @brief   Eclipse Paho ported to ESP8266 RTOS
  *
  ******************************************************************************
  * @copyright
  *
  * Copyright (c) 2015, Baoshi Zhu. All rights reserved.
  * Use of this source code is governed by a BSD-style license that can be
  * found in the LICENSE.txt file.
  *
  * THIS SOFTWARE IS PROVIDED 'AS-IS', WITHOUT ANY EXPRESS OR IMPLIED
  * WARRANTY.  IN NO EVENT WILL THE AUTHOR(S) BE HELD LIABLE FOR ANY DAMAGES
  * ARISING FROM THE USE OF THIS SOFTWARE.
  *
  */
#ifndef _MQTT_ESP8266_H_
#define _MQTT_ESP8266_H_

#include "FreeRTOS.h"
#include "portmacro.h"

typedef struct
{
    portTickType end_time;
} MQTTTimer;

typedef struct _Network
{
	int my_socket;
	int (*mqttread) (struct _Network*, unsigned char*, int, int);
	int (*mqttwrite) (struct _Network*, unsigned char*, int, int);
} Network;

char MQTTTimerExpired(MQTTTimer*);
void MQTTTimerCountDown_ms(MQTTTimer*, unsigned int);
void MQTTTimerCountDown(MQTTTimer*, unsigned int);
int MQTTTimerLeft_ms(MQTTTimer*);

void MQTTTimerInit(MQTTTimer*);

int MQTT_esp_read(Network*, unsigned char*, int, int);
int MQTT_esp_write(Network*, unsigned char*, int, int);
void MQTT_esp_disconnect(Network*);

void NetworkInit(Network* n);
int NetworkConnect(Network* n, const char* host, int port);
int NetworkDisconnect(Network* n);

#endif /* _MQTT_ESP8266_H_ */
