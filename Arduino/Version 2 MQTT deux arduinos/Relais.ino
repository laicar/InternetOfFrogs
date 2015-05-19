/*
 * En cours d'écriture
 * Relais.ino version 2 (Communication MQTT)
 * Contributors: Nicolas MULLER, Guy SINNIG, Carole LAI TONG
 */

#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
//#include <Wire.h>
//#include <RTClib.h>

char hostname[] = "192.168.0.9"; // Adresse IP du broker
int port = 1883; // Port utilisé par le broker
byte mac[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x54 };  // adresse MAC arduino
byte ip[] = { 192,168,0,27 }; // adresse IP arduino récepteur

char printbuf[100];
//int arrivedcount = 0;
EthernetClient c;
IPStack ipstack(c);
//MQTT::Message message;
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

char* manip;
unsigned lum;
float hum;
float temp;

void msgLumArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  /*sprintf(printbuf, "Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n", 
                ++arrivedcount, message.qos, message.retained, message.dup, message.id);
  Serial.print(printbuf);*/
  sprintf(printbuf, "Luminosite %s\n", (char*)message.payload);
  Serial.print(printbuf);
  lum = (unsigned)message.payload;
}

void msgTempArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  sprintf(printbuf, "Temperature %s\n", (char*)message.payload);
  Serial.print(printbuf);
  temp = (((float*)message.payload)[0]);
}

void msgHumiArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  sprintf(printbuf, "Humidite %s\n", (char*)message.payload);
  Serial.print(printbuf);
  hum = (((float*)message.payload)[0]);
}

void afficherRcAnormal(char* msg, int rc, int successRc)
{
  if (rc != successRc)
  Serial.print(msg);
  Serial.println(rc);
}

void msgManipArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  sprintf(printbuf, "Manipulation %s\n", (char*)message.payload);
  Serial.print(printbuf);
  char* delim = {':'};
  manip[0] = (char*)message.payload;
  manip[0] = strtok(manip[0], delim);
  manip[1] = strtok(NULL, delim);
}

// Connexion au Broker MQTT
void connect()
{
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  afficherRcAnormal("TCP connect rc: ", rc, 1);
 
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"arduino-sample2";
  rc = client.connect(data);
  afficherRcAnormal("MQTT connect rc: ", rc, 0);
  
  rc = client.subscribe("InternetOfFrogs/temperature", MQTT::QOS1, msgTempArrived);   
  afficherRcAnormal("MQTT temperature subscribe rc: ", rc, 0);
  rc = client.subscribe("InternetOfFrogs/humidite", MQTT::QOS1, msgHumiArrived);   
  afficherRcAnormal("MQTT humidite subscribe rc: ", rc, 0);
  rc = client.subscribe("InternetOfFrogs/luminosite", MQTT::QOS1, msgLumArrived);   
  afficherRcAnormal("MQTT luminosite subscribe rc: ", rc, 0);
  rc = client.subscribe("InternetOfFrogs/manipulation", MQTT::QOS1, msgManipArrived);   
  afficherRcAnormal("MQTT manipulation subscribe rc: ", rc, 0);
}

void setup()
{
  Serial.begin(9600);
  Ethernet.begin(mac,ip);
  connect();
}
 
void loop()
{
  if (!client.isConnected()) connect();
  client.yield(1000);  
}
