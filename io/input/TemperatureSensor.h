/*
 * TemperatureSensor.h
 *
 *  Created on: 10 août 2015
 *      Author: warole
 */
#include <Observable.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>

#ifndef TEMPERATURESENSOR_H_
#define TEMPERATURESENSOR_H_

class TemperatureSensor: public Observable<FloatInputChangeListener> {
public:
	TemperatureSensor();
	virtual ~TemperatureSensor();
	virtual void update(unsigned long currentTime) = 0;
	virtual float const getState() const = 0;
};

class DHT22SensorAdapter: public TemperatureSensor {
public:
	DHT22SensorAdapter(DHT_Unified * const dht):dht(dht), lastState(0.0), currentState(0.0){}

	~DHT22SensorAdapter(){};

	virtual void update(unsigned long currentTime){
		Serial.println("update()");
		currentState = getState();
		if (currentState != lastState){
			notify(lastState, currentState);
			lastState = currentState;
		}
	}

	virtual float const getState() const {
		DHT_Unified::Temperature temperature = dht->temperature();
		sensors_event_t event;
		temperature.getEvent(&event);
		return event.temperature;
	}

private:
	DHT_Unified * const dht;
	float lastState;
	float currentState;
};



#endif /* TEMPERATURESENSOR_H_ */
