/*
 * MQTTHumidityListener.h
 *
 *  Created on: 21 août 2015
 *      Author: warole
 */

#ifndef IO_LISTENER_MQTTHUMIDITYLISTENER_H_
#define IO_LISTENER_MQTTHUMIDITYLISTENER_H_

#include <MQTTInputChangeListener.h>
#include <MQTTSender.h>

class MQTTHumidityListener: public MQTTInputChangeListener<float> {
public:
	MQTTHumidityListener(MQTT::Client<IPStack, Countdown> * client) :
			MQTTInputChangeListener(client, "InternetOfFrogs/Humidite") {
	}
	virtual ~MQTTHumidityListener();
};

#endif /* IO_LISTENER_MQTTHUMIDITYLISTENER_H_ */
