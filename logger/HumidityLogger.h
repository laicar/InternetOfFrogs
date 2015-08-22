/*
 * HumidityLogger.h
 *
 *  Created on: 22 août 2015
 *      Author: nedjar
 */

#ifndef LOGGER_HUMIDITYLOGGER_H_
#define LOGGER_HUMIDITYLOGGER_H_

#include <InputChangeListener.h>
#include <Arduino.h>

class HumidityLogger: public InputChangeListener<float> {
	virtual void operator()(float const & oldState, float const & newState) {
		Serial.print("humidity: ");
		Serial.println(newState);
	}
};

#endif /* LOGGER_HUMIDITYLOGGER_H_ */
