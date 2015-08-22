
#include "ioFrogs.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <MQTTSender.h>

#include <DHT.h>
#include <Adafruit_TSL2591.h>

#include <TemperatureSensor.h>
#include <HumiditySensor.h>
#include <LuminositySensor.h>

#include <TemperatureLogger.h>
#include <HumidityLogger.h>
#include <LuminosityLogger.h>

#include <MQTTTemperatureListener.h>
#include <MQTTLuminosityListener.h>
#include <MQTTHumidityListener.h>

EthernetClient c;
IPStack * ipstack;
MQTT::Client<IPStack, Countdown>* client;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x76, 0xEC }; // replace with your device's MAC

DHT * dht;
DHTTemperatureSensorAdapter* temperatureSensor;
DHTHumiditySensorAdapter* humiditySensor;

Adafruit_TSL2591* tsl;
TSL2591LuminositySensorAdapter* luminositySensor;

MQTTTemperatureListener* mqttTempListener;
MQTTHumidityListener* mqttHumListener;
MQTTLuminosityListener* mqttLuminosityListener;

TemperatureLogger* tempListener;
HumidityLogger* humListener;
LuminosityLogger* luminosityListener;

void connect() {
	char hostname[] = "192.168.1.4";
	int port = 1883;

	Serial.print("Connecting to ");
	Serial.print(hostname);
	Serial.print(":");
	Serial.println(port);

	int rc = ipstack->connect(hostname, port);
	if (rc != 1) {
		Serial.print("Error. rc from TCP connect is ");
		Serial.println(rc);
	}
	else Serial.println("MQTT connecting");
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = (char*) "arduino-InternetOfFrogs";
	rc = client->connect(data);
	if (rc != 0) {
		Serial.print("Error. rc from MQTT connect is ");
		Serial.println(rc);
	}
	else Serial.println("MQTT connected");
}

void setup() {
	Serial.begin(9600);

	dht = new DHT(2, DHT22);
	dht->begin();

	tsl = new Adafruit_TSL2591(2591);
	tsl->begin();

	temperatureSensor = new DHTTemperatureSensorAdapter(dht);
	tempListener = new TemperatureLogger();
	temperatureSensor->attach(tempListener);

	humiditySensor = new DHTHumiditySensorAdapter(dht);
	humListener = new HumidityLogger();
	humiditySensor->attach(humListener);

	luminositySensor = new TSL2591LuminositySensorAdapter(tsl);
	luminosityListener = new LuminosityLogger();
	luminositySensor->attach(luminosityListener);

	/**/
	Ethernet.begin(mac);
	ipstack = new IPStack(c);
	client = new MQTT::Client<IPStack, Countdown>(*ipstack);

	connect();

	/**/
	mqttTempListener = new MQTTTemperatureListener(client);
	temperatureSensor->attach(mqttTempListener);

	/**/
	mqttHumListener = new MQTTHumidityListener(client);
	humiditySensor->attach(mqttHumListener);
	/**/
	mqttLuminosityListener = new MQTTLuminosityListener(client);
	luminositySensor->attach(mqttLuminosityListener);
	/**/
}

void loop() {
	if (!client->isConnected())
		connect();
	temperatureSensor->update(millis());
	humiditySensor->update(millis());
	luminositySensor->update(millis());
	delay(1000);
}

