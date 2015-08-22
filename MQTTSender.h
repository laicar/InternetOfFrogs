/*
 * MQTTSender.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_INPUT_MQTTSENDER_H_
#define IO_INPUT_MQTTSENDER_H_

#include <MQTTClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <Utils.h>

template<typename StateType>
class MQTTSender {
public:
	static void send(MQTT::Client<IPStack, Countdown> * client,
			const char* const topic, StateType const & value);
};

template<>
class MQTTSender<float> {
public:
	static void send(MQTT::Client<IPStack, Countdown> * client,
			const char* const topic, float const & value) {
		MQTT::Message message;
		char buf[21];
		Utils::floatToCharArray(buf, value, 2);
		message.retained = false;
		message.dup = false;
		message.payload = (void*) buf;
		message.qos = MQTT::QOS1;
		message.payloadlen = strlen(buf) + 1;
		client->publish(topic, message);
	}
};

#endif /* IO_INPUT_MQTTSENDER_H_ */
