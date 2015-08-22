/*
 * MQTTLightListener.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_INPUT_MQTTLUMINOSITYLISTENER_H_
#define IO_INPUT_MQTTLUMINOSITYLISTENER_H_

#include <MQTTInputChangeListener.h>
#include <MQTTSender.h>

class MQTTLuminosityListener: public MQTTInputChangeListener<float> {
public:
	MQTTLuminosityListener(MQTT::Client<IPStack, Countdown> * client) :
			MQTTInputChangeListener(client, "InternetOfFrogs/Lumiere") {
	}
	virtual ~MQTTLuminosityListener();
};

#endif /* IO_INPUT_MQTTLUMINOSITYLISTENER_H_ */
