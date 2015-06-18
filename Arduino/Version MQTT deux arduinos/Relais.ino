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

#define CHAUFFAGE 8
#define BRUMISATEUR 9
#define VENTILATION 10
#define LUMIERE 11
#define POMPE 12

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

void controller (const int appareil, bool& allume,
                const int action, const char* nomAppareil)
{
  if (action == AUTO)
  {
    if (!allume && faitJour()) 
    {
      digitalWrite(appareil,LOW);
      allume = true;
    }
    else if (allume && !faitJour())
    {
      digitalWrite(appareil,HIGH);
      allume = false;
    }
  }
  else if (action == ETEINDRE && allume)
  {
    digitalWrite(appareil, HIGH);
    allume = false;
    Serial1.print("Eteint ");
    Serial1.println(nomAppareil);
  }
  else if (action == ALLUMER && !allume)
  {
    digitalWrite(appareil, LOW);
    allume = true;
    Serial1.print("Allume ");
    Serial1.println(nomAppareil);
  }
  else
  {
    Serial1.print("Erreur avec ");
    Serial1.println(nomAppareil);
  }
}

void lampe(/*const int action*/)
{
  controller (LUMIERE, &lumOn, /*action*/ AUTO, "la lampe");
}

void pompe (const int action)
{
  controller (POMPE, &pompeOn, action, "la pompe a eau");
}

void humidificateur (const int action)
{
  controller (BRUMISATEUR, &fogOn, action, "le brumisateur");
}

void ventilation (const int action)
{
  controller (VENTILATION, &ventOn, action, "la ventilation");
}

void chauffage (const int action)
{
  controller (CHAUFFAGE, &chaufOn, action, "le chauffage");
}

void refroidissement (const int action)
{
  humidificateur(action);
  ventilation(action);
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
  // coupe le message en deux: la fonction à utiliser et l'action à faire
  int appareil = (int)strtok(msg, delim);
  int action = (int)strtok(NULL, delim);
  switch (appareil)
  {
    case CHAUFFAGE:
      chauffage(action);
      break;
    case VENTILATION:
      ventilation(action);
      break;
    case BRUMISATEUR:
      humidificateur(action);
      break;
    case POMPE:
      pompe(action);
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
  pinMode(CHAUFFAGE, OUTPUT); // on va envoyer des ordres sur telle et telle broche
  pinMode(BRUMISATEUR, OUTPUT);
  pinMode(VENTILATION, OUTPUT);
  pinMode(LUMIERE, OUTPUT);
  pinMode(POMPE, OUTPUT);

  rtc.begin(); // mise en route de l'horloge
  pompe (AUTO);
  lampe (/*AUTO*/);
}
 
void loop()
{
  if (!client.isConnected()) connect();
  client.yield(1000);  
}
