/*
 * Relay.h
 *
 *  Created on: 22 août 2015
 *      Author: warole
 */

#ifndef IO_MQTTRELAY_H_
#define IO_MQTTRELAY_H_

#include "Arduino.h"
#include <MQTTClient.h>
#include <MQTTReader.h>

class MQTTRelay {
private:
	typedef void (*messageHandler)(MQTT::MessageData&);

	const int pin;
	bool state;
	const char* const topic;
	MQTT::Client<IPStack, Countdown> * client;
public:
	MQTTRelay(const int pin, MQTT::Client<IPStack, Countdown> * client, const char* const topic): pin(pin), state(), topic(topic), client(client){
		pinMode(pin, OUTPUT);
		this->state = digitalRead(pin);
		MQTTReader::subscribe(this->client, this->topic, mqttDigitalWrite);
	}
	virtual ~MQTTRelay();
	bool getState() {
		this->state = digitalRead(pin);
		return state;
	}
	void mqttDigitalWrite (MQTT::MessageData& md) {
		char* payload = (char*)md.message.payload;
		if (strcmp(payload, "ON") == 0) {
			digitalWrite(this->pin, HIGH);
		}
		else if (strcmp(payload, "OFF") == 0) {
			digitalWrite(this->pin, LOW);
		}
		else {
			Serial.println("Invalid mqttRelay message:");
			Serial.println(payload);
		}
	}
};

#endif /* IO_MQTTRELAY_H_ */
