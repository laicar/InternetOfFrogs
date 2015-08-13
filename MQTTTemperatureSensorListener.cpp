/*
 * MQTTTemperatureSensorListener.cpp
 *
 *  Created on: 10 août 2015
 *      Author: warole
 */

#include <MQTTTemperatureSensorListener.h>

MQTTTemperatureSensorListener::MQTTTemperatureSensorListener(MQTT::Client<IPStack, Countdown> client, const String nomTerrarium) {
	this->client = client;
	String topicStr = "InternetOfFrogs/" + nomTerrarium + "/Temperature";
	this->topic = getBytes(topicStr, topicStr.len);
}

MQTTTemperatureSensorListener::~MQTTTemperatureSensorListener() {}

