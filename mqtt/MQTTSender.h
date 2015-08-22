/*
 * MQTTSender.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_MQTT_MQTTSENDER_H_
#define IO_MQTT_MQTTSENDER_H_

#include <MQTTClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <stdlib.h> // floatToArray()

template<typename StateType>
class MQTTSender {
public:
	/**
		 * Puts a float into a char array.
		 * @param destination - the destination char array
		 * @param value - the float value to write
		 * @param decimals - the number of decimals to write
		 * Truncates the additionnal decimals and
		 * does not handle being asked for more precision than the value already has.
		 * May not give the exact value because of number encoding.
		 */
		static void floatToArray(char* destination, float value, short decimals = 1) {
			long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
			long integerPart = (long)value;
			itoa(integerPart, destination, 10);
			while (*destination != '\0') destination++;
			*destination++ = '.';
			long decimalPart = abs((long)((value - integerPart + 0.000000000) * p[decimals]));
			itoa(decimalPart, destination, 10);
		}
		static void send(MQTT::Client<IPStack, Countdown> * client, const char* const topic, const char* const msg) {
			MQTT::Message message;
			message.retained = false;
			message.dup = false;
			message.payload = (void*)msg;
			message.qos = MQTT::QOS1;
			message.payloadlen = strlen(msg)+1;
			client->publish(topic, message);
		}
		static void send(MQTT::Client<IPStack, Countdown> * client, const char* const topic, float const value) {
			char buf[21];
			floatToArray(buf, value);
			send(client, topic, buf);
		}
};

#endif /* IO_MQTT_MQTTSENDER_H_ */
