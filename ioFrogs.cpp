#include "ioFrogs.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <MQTTSender.h>
#include <MQTTReader.h>

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
#include <MQTTRelay.h>

namespace {
const char* LUMIERE = "lumiere";
const char* FOGGER = "fogger";
const char* POMPE = "pompe";
const char* VENTILATION = "ventilation";
const char* LUMIERECHAUFFANTE = "lumierechauffante";
const char* CHAUFFAGE = "chauffage";

const uint8_t LUMIERE_IDX = 0;
const uint8_t FOGGER_IDX = 1;
const uint8_t POMPE_IDX = 2;
const uint8_t VENTILATION_IDX = 3;
const uint8_t LUMIERECHAUFFANTE_IDX = 4;
const uint8_t CHAUFFAGE_IDX = 5;

const uint8_t nbRelais = 6;

const uint8_t relais[nbRelais] = { LUMIERE_IDX, FOGGER_IDX, POMPE_IDX,
		VENTILATION_IDX, LUMIERECHAUFFANTE_IDX, CHAUFFAGE_IDX };

const char* actionneurs[nbRelais] = { LUMIERE, FOGGER, POMPE, VENTILATION,
		LUMIERECHAUFFANTE, CHAUFFAGE };

char hostname[] = "192.168.1.2";
const int port = 1883;

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
}

void connectToServer() {
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

void connectToMQTT() {
	Serial.print("MQTT connecting...");
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = (char*) "arduino-InternetOfFrogs";
	int rc = mqttClient->connect(data);
	if (rc != 0) {
		Serial.print("Error. rc from MQTT connect is ");
		Serial.println(rc);
	} else
		Serial.println("MQTT connected");
	Serial.println("OK");
}

void connect() {
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
	Serial.println("OK");
}

MQTTRelay* mqttRelay;

//message profile: device:action
//device among chauffage, ventilation, pompe, fogger, lumiere
//action: ON or OFF
void mqttRelayActivate(MQTT::MessageData& md) {
	Serial.println("MQTT relay message received.");
	char* payload = (char*) md.message.payload;
	payload[md.message.payloadlen] = '\0';

	const char* delim = { ":" };

	const char* device = strtok(payload, delim);
	const char* action = strtok(NULL, delim);
	int etat;

	if (strcmp(action, "ON") == 0) {
		etat = HIGH;
	} else if (strcmp(action, "OFF") == 0) {
		etat = LOW;
	} else {
		Serial.print("Invalid action:");
		Serial.println(action);
		return;
	}

	Serial.print("Device : ");
	Serial.print(device);
	Serial.print(", Etat : ");
	Serial.println(etat);

	bool trouve = false;
	for (uint8_t i = 0; i < nbRelais; i++) {
		if (strcmp(device, actionneurs[i]) == 0) {
			digitalWrite(relais[i], etat);
			trouve = true;
		}
	}

	if (!trouve) {
		Serial.println("Invalid device:");
		Serial.println(device);
	}
}

void initializeRelays() {
	for (uint8_t i = 0; i < nbRelais; i++) {
		pinMode(relais[i], OUTPUT);
		digitalWrite(relais[i], LOW);
	}
}

void subscribeRelays() {
	int rc = mqttClient->subscribe("InternetOfFrogs/Relais", MQTT::QOS2,
			mqttRelayActivate);
	if (rc != 0) {
		Serial.print("rc from MQTT subscribe is ");
		Serial.println(rc);
	} else
		Serial.println("MQTT subscribed");
}

void setup() {
	Serial.begin(9600);
	Serial.println("=== Internet Of Frogs ===");
	Serial.println("By Carole Lai Tong and geeks from LAB");
	Serial.println("Sponsorised by W4");
	Serial.println("");

	initializeRelays();
	initializeTemperatureHumiditySensor();
	initializeLuminositySensor();
	initializeSensorAdapters();

	initializeNetwork();
	connect();

	loggersAttachment();
	MQTTListenersAttachment();

	subscribeRelays();

	Serial.println("=========================");
}

void loop() {
	if (!mqttClient->isConnected())
		connect();

	temperatureSensor->update(millis());
	humiditySensor->update(millis());
	luminositySensor->update(millis());

	mqttClient->yield(1000);
}

