#include "ioFrogs.h"

#define WARN Serial.println

#define MQTTCLIENT_QOS2 1

#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <MQTTFloatSender.h>
#include <DHT.h>
#include <TemperatureSensor.h>
#include <HumiditySensor.h>
#include <LuminositySensor.h>
#include <MQTTTemperatureListener.h>
#include <MQTTLightListener.h>
#include <MQTTHumidityListener.h>

EthernetClient c;
IPStack * ipstack;
MQTT::Client<IPStack, Countdown> * client;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x76, 0xEC }; // replace with your device's MAC

DHT * dht;
DHTTemperatureSensorAdapter * temperatureSensor;
DHTHumiditySensorAdapter * humiditySensor;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
TSL2591LuminositySensorAdapter * luminositySensor;

MQTTTemperatureListener * mqttTempObs;
MQTTHumidityListener * mqttHumObs;
MQTTLightListener * mqttLightObs;

class TemperatureLogger: public FloatInputChangeListener {
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("temperature: ");
		Serial.println(newState);
	}
};

TemperatureLogger * tempObs;

class HumidityLogger: public FloatInputChangeListener {
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("humidite: ");
		Serial.println(newState);
	}
};
HumidityLogger * humObs;

class LightLogger: public FloatInputChangeListener {
	virtual void operator()(float const oldState, float const newState) {
		Serial.print("luminosite: ");
		Serial.println(newState);
	}
};
LightLogger * lightObs;

void connect() {
	char hostname[] = "192.168.1.4";
	int port = 1883;

	Serial.print("Connecting to ");
	Serial.print(hostname);
	Serial.print(":");
	Serial.println(port);

	int rc = ipstack->connect(hostname, port);
	if (rc != 1) {
		Serial.print("rc from TCP connect is ");
		Serial.println(rc);
	}

	Serial.println("MQTT connecting");
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = (char*) "arduino-sample";
	rc = client->connect(data);
	if (rc != 0) {
		Serial.print("rc from MQTT connect is ");
		Serial.println(rc);
	}
	Serial.println("MQTT connected");
}

void setup() {
	Serial.begin(9600);
	Ethernet.begin(mac);

	ipstack = new IPStack(c);
	client = new MQTT::Client<IPStack, Countdown>(*ipstack);

	connect();

	dht = new DHT(2, DHT22);
	dht->begin();

	temperatureSensor = new DHTTemperatureSensorAdapter(dht);
	tempObs = new TemperatureLogger();
	temperatureSensor->attach(tempObs);/*
	mqttTempObs = new MQTTTemperatureListener(client);
	temperatureSensor->attach(mqttTempObs);*/

	humiditySensor = new DHTHumiditySensorAdapter(dht);
	humObs = new HumidityLogger();
	humiditySensor->attach(humObs);/*
	mqttHumObs = new MQTTHumidityListener(client);
	humiditySensor->attach(mqttHumObs);*/

	luminositySensor = new TSL2591LuminositySensorAdapter(&tsl);
	lightObs = new LightLogger();
	luminositySensor->attach(lightObs);/*
	mqttLightObs = new MQTTLightListener(client);
	luminositySensor->attach(mqttLightObs);*/
}

MQTT::Message message;

void loop() {
	if (!client->isConnected())
		connect();
	temperatureSensor->update(millis());
	humiditySensor->update(millis());
	luminositySensor->update(millis());
	delay(1000);
}

