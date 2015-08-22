/*
 * MQTTReader.h
 *
 *  Created on: 22 août 2015
 *      Author: warole
 */

#ifndef IO_MQTT_MQTTREADER_H_
#define IO_MQTT_MQTTREADER_H_

#include <MQTTClient.h>
#include <IPStack.h>
#include <Countdown.h>

class MQTTReader {
public:
	typedef void (*messageHandler)(MQTT::MessageData&);

	static void subscribe (MQTT::Client<IPStack, Countdown> * client, const char* const topic, messageHandler treatment){
		int rc = client->subscribe(topic, MQTT::QOS1, treatment);
		if (rc != 0) {
			Serial.print("Error. rc from MQTT subscribe is ");
			Serial.println(rc);
		}
		else Serial.println("MQTT subscribed");
	}

	static void log (MQTT::MessageData& md) {
	  MQTT::Message& message = md.message;
	  Serial.print("Message arrived: qos ");
	  Serial.print(message.qos);
	  Serial.print(", retained ");
	  Serial.print(message.retained);
	  Serial.print(", dup ");
	  Serial.print(message.dup);
	  Serial.print(", packetid ");
	  Serial.println(message.id);
	  Serial.print("Payload ");
	  Serial.println((char*)message.payload);
	}
};

#endif /* IO_MQTT_MQTTREADER_H_ */
