/*
 * MQTTTemperatureSensorListener.h
 *
 *  Created on: 10 août 2015
 *      Author: warole
 */
#include <InputChangeListener.h>
#include <MQTTClient.h>
#include <IPStack.h>
#include <Countdown.h>

#ifndef MQTTTEMPERATURESENSORLISTENER_H_
#define MQTTTEMPERATURESENSORLISTENER_H_

class MQTTTemperatureSensorListener: public FloatInputChangeListener {
private:
	MQTT::Client<IPStack, Countdown> client;
	char* topic;
public:
	MQTTTemperatureSensorListener(MQTT::Client<IPStack, Countdown> client, String nomTerrarium):client(client), topic(NULL) {
		String topicStr = "InternetOfFrogs/" + nomTerrarium + "/Temperature";
		topicStr.toCharArray(this->topic, topicStr.length()+1);
	}

	virtual ~MQTTTemperatureSensorListener();
	virtual void operator()(float const oldState, float const newState) {
		MQTT::Message message;
		char* buf;
		sprintf(buf, "%f", (double)newState); // met la valeur dans un tampon
		message.retained = false;
		message.dup = false;
		message.payload = (void*)buf;
		message.qos = MQTT::QOS1;
		message.payloadlen = strlen(buf)+1;
		client.publish(topic, message);
	}
};

#endif /* MQTTTEMPERATURESENSORLISTENER_H_ */
