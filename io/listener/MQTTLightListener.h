/*
 * MQTTLightListener.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_INPUT_MQTTLIGHTLISTENER_H_
#define IO_INPUT_MQTTLIGHTLISTENER_H_

#include <InputChangeListener.h>
#include <MQTTFloatSender.h>

class MQTTLightListener: public FloatInputChangeListener {
private:
	MQTT::Client<IPStack, Countdown> client;
	char* topic;
public:
	MQTTLightListener(MQTT::Client<IPStack, Countdown> client, String nomTerrarium):client(client), topic() {
		String topicStr = "InternetOfFrogs/" + nomTerrarium + "/Lumiere";
		topicStr.toCharArray(this->topic, topicStr.length()+1);
	}
	virtual ~MQTTLightListener();
	virtual void operator()(float const oldState, float const newState) {
		MQTTFloatSender::send(client, topic, newState);
	}
};

#endif /* IO_INPUT_MQTTLIGHTLISTENER_H_ */
