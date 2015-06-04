/*
 * Relais.ino
 * Communication MQTT 
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
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x76, 0xEC };  // adresse MAC de l'arduino
byte ip[] = { 192,168,0,27 }; // adresse IP de l'arduino

EthernetClient c;
IPStack ipstack(c);
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

#define CHAUFF 8
#define FOGGER 9
#define VENT 10
#define LUMIERE 11

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
  return (now.hour() >= H_MATIN && now.hour() < H_SOIR);
}

void lampe ()
{
  if (faitJour()) digitalWrite(LUMIERE,LOW);
  else digitalWrite(LUMIERE,HIGH);
  
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
  else if (temp > 25.0 || (!(faitJour()) && temp > 23.0))
    chauffage(ETEINDRE);
  else if (temp > 26.0 || (!(faitJour()) && temp > 24.0))
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

void messageManipArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  sprintf(printbuf, "Manipulation %s\n", (char*)message.payload);
  Serial1.print(printbuf);//affiche le message sur le port série
  char* msg = (char*)message.payload; //stocke le message
  const char* delim = {":"};
  int appareil = (int)strtok(msg, delim);// coupe le message en deux: la fonction à utiliser...
  int action = (int)strtok(NULL, delim); // et l'action à faire
  switch (appareil)
  {
    case CHAUFF:
      chauffage(action);
      break;
    case VENT:
      ventilation(action);
      break;
    case FOGGER:
      humidificateur(action);
      break;
  }
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
  client.subscribe("InternetOfFrogs/Terrarium1/Manipulation", MQTT::QOS1, msgManipArrived);
}

void setup()
{
  Ethernet.begin(mac,ip);
  connect();
  pinMode(CHAUFF, OUTPUT); // on va envoyer des ordres sur telle et telle broche
  pinMode(FOGGER, OUTPUT);
  pinMode(VENT, OUTPUT);
}
 
void loop()
{
  if (!client.isConnected()) connect();
  client.yield(1000);  
}
