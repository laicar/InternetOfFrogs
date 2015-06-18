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
#define AUTO 2

RTC_DS1307 rtc;
bool chaufOn = false;
bool fogOn = true;
bool ventOn = true;
bool pompeOn = false;
bool lumOn = false;

const char* topicManipulations = "InternetOfFrogs/Terrarium1/Manipulation";

char buf[200]; //Tampon pour envoyer et recevoir des messages

bool faitJour ()
{
  DateTime now = rtc.now(); // Récupère la date et l'heure actuelles
  return (now.hour() >= H_MATIN && now.hour() < H_SOIR);
}

void controller (const int appareil, bool& allume, const int action)
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
  }
  else if (action == ALLUMER && !allume)
  {
    digitalWrite(appareil, LOW);
    allume = true;
  }
}

void lampe(/*const int action*/)
{
  controller (LUMIERE, lumOn, /*action*/ AUTO);
}

void pompe (const int action)
{
  controller (POMPE, pompeOn, action);
}

void humidificateur (const int action)
{
  controller (BRUMISATEUR, fogOn, action);
}

void ventilation (const int action)
{
  controller (VENTILATION, ventOn, action);
}

void chauffage (const int action)
{
  controller (CHAUFFAGE, chaufOn, action);
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

void messageTempArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  reguleTemp((((float*)message.payload)[0]));
}

void messageHumiArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  reguleHum((((float*)message.payload)[0]));
}

void messageManipArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  char* msg = (char*)message.payload; //stocke le message
  Serial.print("Manipulation ");
  Serial.println(msg);
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

void afficherRcAnormal(char* msg, int rc, int successRc)
{
  if (rc != successRc)
  Serial.print(msg);
  Serial.println(rc);
}

// Connexion au Broker MQTT
void connect()
{
  sprintf(buf, "Connexion à %s:%d\n", hostname, port);
  Serial.print(buf);
  int rc = ipstack.connect(hostname, port); // tentative de connexion
  afficherRcAnormal("TCP connect rc: ", rc, 1);
 
  Serial.println("Connexion a MQTT");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"Terrarium1";
  rc = client.connect(data);
  afficherRcAnormal("MQTT connect rc: ", rc, 0);

  Serial.print("Souscrit a ");
  Serial.println(topicManipulations);
  rc = client.subscribe(topicManipulations, MQTT::QOS1, messageManipArrived);
  afficherRcAnormal("MQTT manipulation subscribe rc: ", rc, 0);
}

void setup()
{
  Serial.begin(9600);
  
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
