/*
 * LuminosityLogger.h
 *
 *  Created on: 22 août 2015
 *      Author: nedjar
 */

#ifndef LOGGER_LUMINOSITYLOGGER_H_
#define LOGGER_LUMINOSITYLOGGER_H_

#include <InputChangeListener.h>
#include <Arduino.h>

class LuminosityLogger: public InputChangeListener<float> {
	virtual void operator()(float const & oldState, float const & newState) {
		Serial.print("luminosity: ");
		Serial.println(newState);
	}
};

#endif /* LOGGER_LUMINOSITYLOGGER_H_ */
