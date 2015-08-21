/*
 * LuminositySensor.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */

#ifndef IO_INPUT_LUMINOSITYSENSOR_H_
#define IO_INPUT_LUMINOSITYSENSOR_H_

#include <Observable.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_TSL2591.h>

class LuminositySensor: public Observable<FloatInputChangeListener, float> {
public:
	LuminositySensor();
	virtual ~LuminositySensor();virtual void update(unsigned long currentTime) = 0;
	virtual float const getState() const = 0;
};

class TSL2591LuminositySensorAdapter: public LuminositySensor {
public:
	TSL2591LuminositySensorAdapter(Adafruit_TSL2591 * const tsl): tsl(tsl),lastState(0.0), currentState(0.0){
	}

	~TSL2591LuminositySensorAdapter(){};

	virtual void update(unsigned long currentTime){
		currentState = getState();
		if (currentState != lastState){
			notify(lastState, currentState);
			lastState = currentState;
		}
	}

	virtual float const getState() const {
		sensors_event_t event;
		tsl->getEvent(&event);
		return event.light;
	}

private:
	Adafruit_TSL2591 * const tsl;
	float lastState;
	float currentState;
};

#endif /* IO_INPUT_LUMINOSITYSENSOR_H_ */
