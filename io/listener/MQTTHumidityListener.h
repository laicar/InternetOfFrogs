/*
 * MQTTHumidityListener.h
 *
 *  Created on: 21 août 2015
 *      Author: warole
 */

#ifndef IO_LISTENER_MQTTHUMIDITYLISTENER_H_
#define IO_LISTENER_MQTTHUMIDITYLISTENER_H_

#include <InputChangeListener.h>
#include <MQTTPublisher.h>

class MQTTHumidityListener: public FloatInputChangeListener {
private:
	MQTT::Client<IPStack, Countdown> * client;
	char* topic;
public:
	MQTTHumidityListener(MQTT::Client<IPStack, Countdown> * client) :
			client(client), topic("InternetOfFrogs/Humidite") {
		//topicStr.toCharArray(this->topic, topicStr.length()+1);
		Serial.println("creation mqtt temp listener");
		Serial.println(this->topic);
	}
	virtual ~MQTTHumidityListener();
	virtual void operator()(float const oldState, float const newState) {
		MQTTPublisher::send(client, topic, newState);
	}
};

#endif /* IO_LISTENER_MQTTHUMIDITYLISTENER_H_ */
