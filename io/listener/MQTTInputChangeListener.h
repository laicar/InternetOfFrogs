/*
 * MQTTInputChangeListener.h
 *
 *  Created on: 22 août 2015
 *      Author: nedjar
 */

#ifndef IO_LISTENER_MQTTINPUTCHANGELISTENER_H_
#define IO_LISTENER_MQTTINPUTCHANGELISTENER_H_

#include <InputChangeListener.h>
#include <MQTTSender.h>

template<typename StateType>
class MQTTInputChangeListener: public InputChangeListener<StateType> {
private:
	MQTT::Client<IPStack, Countdown> * client;
	const char* topic;
public:
	MQTTInputChangeListener(MQTT::Client<IPStack, Countdown> * client,
			const char* topic) :
			client(client), topic(topic) {
		Serial.print("creation mqtt listener for the topic : ");
		Serial.println(this->topic);
	}

	virtual ~MQTTInputChangeListener() {
	}

	virtual void operator()(StateType const & oldState,
			StateType const & newState) {
		MQTTSender::send(client, topic, newState);
	}
};

#endif /* IO_LISTENER_MQTTINPUTCHANGELISTENER_H_ */
