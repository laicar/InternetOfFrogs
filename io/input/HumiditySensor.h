/*
 * HumiditySensor.h
 *
 *  Created on: 20 août 2015
 *      Author: nedjar
 */
#ifndef HUMIDITYSENSOR_H_
#define HUMIDITYSENSOR_H_

#include <Observable.h>
#include <DHT.h>
#include <InputChangeListener.h>

class HumiditySensor: public Observable<FloatInputChangeListener, float>  {
public:
	HumiditySensor();
	virtual ~HumiditySensor();
	virtual void update(unsigned long currentTime) = 0;
	virtual float const getState() const = 0;
};

class DHTHumiditySensorAdapter: public HumiditySensor {
public:
	DHTHumiditySensorAdapter(DHT * const dht):dht(dht), lastState(0.0), currentState(0.0){}

	~DHTHumiditySensorAdapter(){};

	virtual void update(unsigned long currentTime){
		currentState = getState();
		if (currentState != lastState){
			notify(lastState, currentState);
			lastState = currentState;
		}
	}

	virtual float const getState() const {
		return dht->readHumidity();
	}

private:
	DHT * const dht;
	float lastState;
	float currentState;
};

#endif /* HUMIDITYSENSOR_H_ */
