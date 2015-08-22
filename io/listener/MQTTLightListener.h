/*
 * MQTTLightListener.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_INPUT_MQTTLIGHTLISTENER_H_
#define IO_INPUT_MQTTLIGHTLISTENER_H_

#include <InputChangeListener.h>
#include <MQTTPublisher.h>

class MQTTLightListener: public FloatInputChangeListener {
private:
	MQTT::Client<IPStack, Countdown> * client;
	char* topic;
public:
	MQTTLightListener(MQTT::Client<IPStack, Countdown> * client) :
			client(client), topic("InternetOfFrogs/Lumiere") {
		//String topicStr = "InternetOfFrogs/" + nomTerrarium + "/Lumiere";
		//topicStr.toCharArray(this->topic, topicStr.length()+1);
		Serial.println("creation mqtt temp listener");
		Serial.println(this->topic);
	}
	virtual ~MQTTLightListener();
	virtual void operator()(float const oldState, float const newState) {
		MQTTPublisher::send(client, topic, newState);
	}
};

#endif /* IO_INPUT_MQTTLIGHTLISTENER_H_ */
