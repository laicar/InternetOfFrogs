/**************************
 * Terrarium_MQTT_compact.ino
 * Programme utilisant capteurs et relais pour maintenir 
 * des conditions tropicales. Utilise le protocole MQTT pour
 * envoyer les valeurs captées et recevoir des commandes via IP
 * Contributeur: Carole LAI TONG
 **************************/
 
/**************************
 * Fichiers des librairies utilisées dans ce programme.
 * Déclarent des valeurs et des fonctions
 **************************/
#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h> // Communication MQTT
#include <Wire.h>
#include <RTClib.h> // Horloge
#include <Adafruit_Sensor.h> // Capteurs Adafruit unifiés
#include <Adafruit_TSL2591.h> // Capteur de lumière TSL2591
#include <DHT.h> // Capteur humidité/température
#include <DHT_U.h> // Capteur humidité/température unifié

/**************************
 * Déclaration et initialisation des variables
 * Avec #define, toute instance du premier mot
 * sera remplacée par le deuxième dans le code
 **************************/

// Variables Capteurs
// On utilise un capteur humidité/température DHT22
// dont la borne data est branchée sur la broche 2
DHT_Unified dht(2, DHT22);
// on indique le type de capteur de luminosité
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

// Variables Relais
// Bornes sur lesquelles les cables du relais
// correspondant à chaque appareil seront branchés
#define CHAUFFAGE 8
#define BRUMISATEUR 9
#define VENTILATION 10
#define LUMIERE 11
#define POMPE 12
//#define NOURRITURE 13

#define H_MATIN 7
#define H_SOIR 19

#define ALLUMER 0
#define ETEINDRE 1
#define AUTO 2

RTC_DS1307 rtc;
bool chaufOn = false;
bool fogOn = false;
bool ventOn = false;
bool pompeOn = false;
bool lumOn = false;

// Variables MQTT
char hostname[] = "192.168.0.9";// Adresse IP du broker
int port = 1883;// Port utilisé par le broker
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x76, 0xEC }; // Adresse MAC de l'arduino
byte ip[] = { 192,168,0,28 }; // adresse IP de l'arduino

// Noms des topics où on publie des messages
const char* topicTemperature = "InternetOfFrogs/Terrarium1/Valeur/Temperature";
const char* topicHumidite = "InternetOfFrogs/Terrarium1/Valeur/Humidite";
const char* topicLuminosite = "InternetOfFrogs/Terrarium1/Valeur/Luminosite";
// Noms des topics où on écoute des messages
const char* topicManipulation = "InternetOfFrogs/Terrarium1/Manipulation";

char buf[200]; //Tampon pour envoyer et recevoir des messages
EthernetClient c;
IPStack ipstack (c);
MQTT::Message message; // crée un message MQTT (pour l'instant vide)
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

/**************************
 * Methodes
 **************************/

 // Methodes pour la régulation du terrarium
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

// Methodes pour la communication MQTT

// Envoie une valeur sur le topic adequat
void pushData(float valeur, const char* topic)
{
     message.retained = false;
     message.dup = false;
     message.payload = (void*)buf;
     message.qos = MQTT::QOS1;
     message.payloadlen = strlen(buf)+1;
     client.publish(topic, message);
}

// Reception d'un ordre de manipulation
void messageManipArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  char* msg = (char*)message.payload; //stocke le message
  const char* delim = {":"};
  // coupe le message en deux: l'appareil et l'action à faire
  const int appareil = (int)strtok(msg, delim);
  const int action = (int)strtok(NULL, delim);
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

// Fonction de connexion au broker MQTT
void connect()
{
  ipstack.connect(hostname, port); // tentative de connexion au broker
 
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"Terrarium1";
  client.connect(data);
  client.subscribe(topicManipulation, MQTT::QOS1, messageManipArrived);
}

/**************************
 * La fonction setup d'Arduino s'effectue une seule fois après la remise à zéro
 **************************/
void setup()
{
  Ethernet.begin(mac,ip);
  connect(); // connexion au broker

  // on va envoyer des ordres sur telle et telle broche
  pinMode(CHAUFFAGE, OUTPUT);
  pinMode(BRUMISATEUR, OUTPUT);
  pinMode(VENTILATION, OUTPUT);
  pinMode(LUMIERE, OUTPUT);
  pinMode(POMPE, OUTPUT);

  rtc.begin(); // mise en route de l'horloge
  dht.begin(); // mise en route du capteur humidité/température
}

/**************************
 * La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
 **************************/
void loop()
{
  lampe(/*AUTO*/);
  pompe (AUTO);
  
  sensors_event_t event;
  
  // Lumière
  tsl.getEvent(&event);
  if ((event.light != 0.0) &&
      (event.light <= 4294966000.0) && 
      (event.light >= -4294966000.0))
  {
/* If event.light = 0 lux the sensor is probably saturated
 * and no reliable data could be generated!
 * if event.light is +/- 4294967040 there was a float over/underflow */
// envoie la valeur captée sur le topic correspondant si c'est bon
    pushData(event.light, topicLuminosite);
  }
  
  // Température
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
  {
    pushData(event.temperature, topicTemperature);
    reguleTemp(event.temperature);
  }

  // Humidite
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    pushData(event.relative_humidity, topicHumidite);
    reguleHum(event.relative_humidity);
  }
  delay(600000); // attente de 10 minutes avant de recommencer la boucle
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte
}
