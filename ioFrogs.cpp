
#include "ioFrogs.h"
#include <TemperatureSensor.h>
#include <HumiditySensor.h>
#include <LuminositySensor.h>
#include <MQTTLightListener.h>
#include <EthernetClient.h>
#include <IPStack.h>

//DHT_Unified dht (2, DHT11);
//DHTTemperatureSensorAdapter temperatureSensor (&dht);
//DHTHumiditySensorAdapter humiditySensor (&dht);
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
TSL2591LuminositySensorAdapter luminositySensor(&tsl);
EthernetClient c;
IPStack ipstack (c);
MQTT::Message message; // crée un message MQTT
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);
MQTTLightListener testObservateurLuminositeMQTT = MQTTLightListener (client, "reblochonrium");
/*
class TestObservateurTemperature : public FloatInputChangeListener
{
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("temperature: ");
		Serial.println(newState);
	}
};

TestObservateurTemperature testObservateurTemperature;

class TestObservateurHumidite : public FloatInputChangeListener
{
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("humidite: ");
		Serial.println(newState);
	}
};

TestObservateurHumidite testObservateurHumidite;

class TestObservateurLuminosite : public FloatInputChangeListener
{
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("luminosite: ");
		Serial.println(newState);
	}
};

TestObservateurLuminosite testObservateurLuminosite;
*/
void setup()
{
	Serial.begin(9600);
	Serial.println("Frogs?");
	//dht.begin();
	tsl.begin();
	tsl.setGain(TSL2591_GAIN_MED);
	tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
	//temperatureSensor.attach(&testObservateurTemperature);
	//humiditySensor.attach(&testObservateurHumidite);
	luminositySensor.attach(&testObservateurLuminositeMQTT);
}

void loop()
{
	//temperatureSensor.update(millis());
	//humiditySensor.update(millis());
	luminositySensor.update(millis());
	delay(500);
}
