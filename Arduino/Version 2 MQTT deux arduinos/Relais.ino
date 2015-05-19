/*
 * Relais.ino v2 (Communication MQTT entre un capteur et un relais)
 * Contributors: Nicolas MULLER, Guy SINNIG, Carole LAI TONG
 */

#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <RTClib.h>

char hostname[] = "192.168.0.9"; // Adresse IP du broker
int port = 1883; // Port utilisé par le broker
byte mac[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x54 };  // adresse MAC arduino
byte ip[] = { 192,168,0,27 }; // adresse IP arduino récepteur

EthernetClient c;
IPStack ipstack(c);
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

#define CHAUFF 8
#define FOGGER 9
#define VENT 10
//#define POMPE 11
//#define LUMIERE 12
//#define NOURRITURE 13

#define H_MATIN 7
#define H_SOIR 19

#define ALLUMER 0
#define ETEINDRE 1

RTC_DS1307 rtc;
bool chaufOn = false;
bool fogOn = true;
bool ventOn = true;

bool faitJour ()
{
  DateTime now = rtc.now(); // Récupère la date et l'heure actuelles
  return (now.hour() > H_MATIN && now.hour() < H_SOIR);
}

void chauffage (const int action)
{
  if (action == ALLUMER && !chaufOn)
  {
    digitalWrite(CHAUFF,LOW);
    chaufOn = true;
  }
  else if (action == ETEINDRE && chaufOn)
  {
    digitalWrite(CHAUFF,HIGH);
    chaufOn = false;
  }
}

void refroidissement (const int action)
{
  if (action == ALLUMER)
  {
    humidificateur(ALLUMER);
    ventilation(ALLUMER);
  }
  else if (action == ETEINDRE)
  {
    humidificateur(ETEINDRE);
    ventilation(ETEINDRE);
  }
}

void ventilation (const int action)
{
  if (action == ALLUMER && !ventOn)
  {
    digitalWrite(VENT,LOW);
    ventOn = true;
  }
  else if (action == ETEINDRE && ventOn)
  {
    digitalWrite(VENT,HIGH);
    ventOn = false;
  }
}

void humidificateur (const int action)
{
  if (action == ALLUMER && !fogOn)
  {
    digitalWrite(FOGGER,LOW);
    fogOn = true;
  }
  else if (action == ETEINDRE && fogOn)
  {
    digitalWrite(FOGGER,HIGH);
    fogOn = false;
  }
}


// /!\ Il y a des chances que ça se bloque quelquepart
// entre le refroidissement et la régulation de l'humidité
// et il faudrait peut-etre mettre de la ventilation périodique automatique


void reguleTemp (float temp)
{
  if (temp < 22.0 || (faitJour() && temp < 24.0))
    chauffage(ALLUMER);
  else if (fogOn && ventOn &&
           (temp < 22.5 || (faitJour() && temp < 24.5)))
    refroidissement(ETEINDRE);
  else if (temp > 25.5 || (!(faitJour()) && temp > 23.5))
    chauffage(ETEINDRE);
  else if (temp > 26.8 || (!(faitJour()) && temp > 24.8))
    refroidissement(ALLUMER);
}

void reguleHum (float hum)
{
  if (hum < 69.0)
    humidificateur(ALLUMER);
  else if (hum < 72.0)
    ventilation(ETEINDRE);
  else if (hum > 78.0)
    humidificateur(ETEINDRE);
  else if (hum > 81.0)
    ventilation(ALLUMER);
}

void msgTempArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  reguleTemp((((float*)message.payload)[0]));
}

void msgHumiArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  reguleHum((((float*)message.payload)[0]));
}

// Connexion au Broker MQTT
void connect()
{
  ipstack.connect(hostname, port);
 
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"arduino-relais";
  client.connect(data);
  
  client.subscribe("InternetOfFrogs/Terrarium1/Valeur/Temperature", MQTT::QOS1, msgTempArrived);
  client.subscribe("InternetOfFrogs/Terrarium1/Valeur/Humidite", MQTT::QOS1, msgHumiArrived);
}

void setup()
{
  Ethernet.begin(mac,ip);
  connect();
}
 
void loop()
{
  if (!client.isConnected()) connect();
  client.yield(1000);  
}
