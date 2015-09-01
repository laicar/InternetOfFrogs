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

#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor: public Observable<InputChangeListener<float>, float> {
public:
	TemperatureSensor();
	virtual ~TemperatureSensor();
	virtual void update(unsigned long currentTime) = 0;
	virtual float const getState() const = 0;
};

class DHTTemperatureSensorAdapter: public TemperatureSensor {
public:
	DHTTemperatureSensorAdapter(DHT * const dht) :
			dht(dht), lastState(0.0), currentState(0.0) {
	}

	~DHTTemperatureSensorAdapter() {
	}
	;

	virtual void update(unsigned long currentTime) {
		currentState = getState();
		if (currentState != lastState) {
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

class DS18B20TemperatureSensorAdapter: public TemperatureSensor {
public:
	DS18B20TemperatureSensorAdapter(DallasTemperature * const sensors, const uint8_t index) :
			sensors(sensors), index(index), lastState(0.0), currentState(0.0) {
	}

	~DS18B20TemperatureSensorAdapter() {};

	virtual void update(unsigned long currentTime) {
		currentState = getState();
		if (currentState != lastState) {
			notify(lastState, currentState);
			lastState = currentState;
		}
	}

	virtual float const getState() const {
		Serial.print("Requesting temperatures...");
		sensors->requestTemperatures(); // Send the command to get temperatures
		Serial.println("DONE");

		Serial.print("Temperature for Device ");
		Serial.print(index)
		Serial.print(" is: ");
		Serial.println(sensors->getTempCByIndex(index));

		return sensors->getTempCByIndex(0);
	}

private:
	DallasTemperature * const sensors;
	const uint8_t index;
	float lastState;
	float currentState;
};

#endif /* TEMPERATURESENSOR_H_ */
