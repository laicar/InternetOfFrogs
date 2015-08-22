/*
 * MQTTTemperatureListener.h
 *
 *  Created on: 21 août 2015
 *      Author: warole
 */

#ifndef IO_LISTENER_MQTTTEMPERATURELISTENER_H_
#define IO_LISTENER_MQTTTEMPERATURELISTENER_H_


#include <MQTTInputChangeListener.h>
#include <MQTTSender.h>

class MQTTTemperatureListener: public MQTTInputChangeListener<float> {
public:
	MQTTTemperatureListener(MQTT::Client<IPStack, Countdown> * client) :
			MQTTInputChangeListener(client, "InternetOfFrogs/Temperature") {
	}
	virtual ~MQTTTemperatureListener();
};

#endif /* IO_LISTENER_MQTTTEMPERATURELISTENER_H_ */
