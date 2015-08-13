
#include "ioFrogs.h"
#include <TemperatureSensor.h>

DHT_Unified dht (2, DHT22);
DHT22SensorAdapter temperatureSensor (&dht);

class TestObservateurTemperature : public FloatInputChangeListener
{
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("coucou(");
		Serial.print(oldState);
		Serial.print(", ");
		Serial.print(newState);
		Serial.println(")");
	}
};

TestObservateurTemperature *test;

void setup()
{
	Serial.begin(9600);
	dht.begin();

	test = new TestObservateurTemperature();

	Serial.println("begin");
	temperatureSensor.attach(test);
}

void loop()
{
	temperatureSensor.update(millis());

}
