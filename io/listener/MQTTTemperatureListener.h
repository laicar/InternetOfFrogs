/*
 * MQTTTemperatureListener.h
 *
 *  Created on: 21 août 2015
 *      Author: warole
 */

#ifndef IO_LISTENER_MQTTTEMPERATURELISTENER_H_
#define IO_LISTENER_MQTTTEMPERATURELISTENER_H_

#include <InputChangeListener.h>
#include <MQTTPublisher.h>

class MQTTTemperatureListener: public FloatInputChangeListener {
private:
	MQTT::Client<IPStack, Countdown> * client;
	char* topic;
public:
	MQTTTemperatureListener(MQTT::Client<IPStack, Countdown> * client) :
			client(client), topic("InternetOfFrogs/Temperature") {
		Serial.println("creation mqtt temp listener");
		Serial.println(this->topic);
	}
	virtual ~MQTTTemperatureListener();
	virtual void operator()(float const oldState, float const newState) {
		MQTTPublisher::send(client, topic, newState);
	}
};

#endif /* IO_LISTENER_MQTTTEMPERATURELISTENER_H_ */
