/*
 * TemperatureLogger.h
 *
 *  Created on: 22 août 2015
 *      Author: nedjar
 */

#ifndef LOGGER_TEMPERATURELOGGER_H_
#define LOGGER_TEMPERATURELOGGER_H_

#include <InputChangeListener.h>
#include <Arduino.h>

class TemperatureLogger: public InputChangeListener<float> {
	virtual void operator()(float const & oldState, float const & newState) {
		Serial.print("temperature: ");
		Serial.println(newState);
	}
};

#endif /* LOGGER_TEMPERATURELOGGER_H_ */
