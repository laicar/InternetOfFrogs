
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

EthernetClient* ethernetClient;
IPStack* ipstack;
MQTT::Client<IPStack, Countdown>* mqttClient;

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

void connectToServer() {
	char hostname[] = "192.168.1.4";
	int port = 1883;

	Serial.print("Connecting to ");
	Serial.print(hostname);
	Serial.print(":");
	Serial.print(port);
	Serial.print("...");

	int rc = ipstack->connect(hostname, port);
	if (rc != 1) {
		Serial.print("Error. rc from TCP connect is ");
		Serial.println(rc);
	}
	Serial.println("OK");
}

void connectToMQTT(){
	Serial.print("MQTT connecting...");
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = (char*) "terrarium";
	int rc = mqttClient->connect(data);
	if (rc != 0) {
		Serial.print("Error. rc from MQTT connect is ");
		Serial.println(rc);
	}
	else Serial.println("MQTT connected");
}

void connect(){
	connectToServer();
	connectToMQTT();
}

void MQTTListenersAttachment() {
	Serial.println("MQTT Listener attachment...");
	mqttTempListener = new MQTTTemperatureListener(mqttClient);
	temperatureSensor->attach(mqttTempListener);
	mqttHumListener = new MQTTHumidityListener(mqttClient);
	humiditySensor->attach(mqttHumListener);
	mqttLuminosityListener = new MQTTLuminosityListener(mqttClient);
	luminositySensor->attach(mqttLuminosityListener);
	Serial.println("OK");
}

void loggersAttachment() {
	Serial.print("Loggers attachment...");
	tempListener = new TemperatureLogger();
	temperatureSensor->attach(tempListener);
	humListener = new HumidityLogger();
	humiditySensor->attach(humListener);
	luminosityListener = new LuminosityLogger();
	luminositySensor->attach(luminosityListener);
	Serial.println("OK");
}

void initializeSensorAdapters() {
	Serial.print("Sensor Adapter Initialisation...");
	temperatureSensor = new DHTTemperatureSensorAdapter(dht);
	humiditySensor = new DHTHumiditySensorAdapter(dht);
	luminositySensor = new TSL2591LuminositySensorAdapter(tsl);
	Serial.println("OK");
}

void initializeLuminositySensor() {
	Serial.print("TSL2591 Initialisation...");
	tsl = new Adafruit_TSL2591(2591);
	tsl->begin();
	Serial.println("OK");
}

void initializeTemperatureHumiditySensor() {
	/**/
	Serial.print("DHT Initialisation...");
	dht = new DHT(2, DHT11);
	dht->begin();
	Serial.println("OK");
}

void initializeNetwork() {
	/**/
	Serial.print("Etherner Shield Initialisation...");
	Ethernet.begin(mac);
	ethernetClient = new EthernetClient();
	ipstack = new IPStack(*ethernetClient);
	mqttClient = new MQTT::Client<IPStack, Countdown>(*ipstack);
	Serial.println("OK\n");
}

void setup() {
	Serial.begin(9600);
	Serial.println("=== Internet Of Frogs ===");
	Serial.println("By Carole Lai Tong and geeks from LAB");
	Serial.println("Sponsorised by W4");
	Serial.println("");

	initializeTemperatureHumiditySensor();
	initializeLuminositySensor();
	initializeSensorAdapters();
	loggersAttachment();

	initializeNetwork();
	connect();
	MQTTListenersAttachment();
	Serial.println("=========================");
}

void loop() {
	if (!mqttClient->isConnected())
		connect();

	temperatureSensor->update(millis());
	humiditySensor->update(millis());
	luminositySensor->update(millis());
	delay(1000);
}

