/*
 * TemperatureSensor.h
 *
 *  Created on: 10 août 2015
 *      Author: warole
 */
#ifndef TEMPERATURESENSOR_H_
#define TEMPERATURESENSOR_H_

#include <Observable.h>
#include <InputChangeListener.h>
#include <DHT.h>

class TemperatureSensor: public Observable<FloatInputChangeListener, float> {
public:
	TemperatureSensor();
	virtual ~TemperatureSensor();
	virtual void update(unsigned long currentTime) = 0;
	virtual float const getState() const = 0;
};

class DHTTemperatureSensorAdapter: public TemperatureSensor {
public:
	DHTTemperatureSensorAdapter(DHT * const dht):dht(dht), lastState(0.0), currentState(0.0){}

	~DHTTemperatureSensorAdapter(){};

	virtual void update(unsigned long currentTime){
		currentState = getState();
		if (currentState != lastState){
			notify(lastState, currentState);
			lastState = currentState;
		}
	}

	virtual float const getState() const {
		return dht->readTemperature();
	}

private:
	DHT * const dht;
	float lastState;
	float currentState;
};



#endif /* TEMPERATURESENSOR_H_ */
