/**************************
 * Terrarium_MQTT.ino
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

// Noms des topics où on publie ou lit des messages
const char* topicTemperature = "InternetOfFrogs/Terrarium1/Valeur/Temperature";
const char* topicHumidite = "InternetOfFrogs/Terrarium1/Valeur/Humidite";
const char* topicLuminosite = "InternetOfFrogs/Terrarium1/Valeur/Luminosite";
const char* topicManipulations = "InternetOfFrogs/Terrarium1/Manipulation";

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
  controller (LUMIERE, lumOn, /*action*/ AUTO, "la lampe");
}

void pompe (const int action)
{
  controller (POMPE, pompeOn, action, "la pompe a eau");
}

void humidificateur (const int action)
{
  controller (BRUMISATEUR, fogOn, action, "le brumisateur");
}

void ventilation (const int action)
{
  controller (VENTILATION, ventOn, action, "la ventilation");
}

void chauffage (const int action)
{
  controller (CHAUFFAGE, chaufOn, action, "le chauffage");
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

void afficherRcAnormal(char* msg, int rc, int successRc)
{
  if (rc != successRc)
  Serial1.print(msg);
  Serial1.println(rc);
}

// Envoie une valeur sur le topic adequat
void pushData(float valeur, const char* topic)
{
     sprintf(buf, "%f", valeur); // met la valeur dans le tampon buf
     Serial1.println(buf); // envoie le contenu du tampon buf sur le port série
     message.retained = false;
     message.dup = false;
     message.payload = (void*)buf;
     message.qos = MQTT::QOS1;
     message.payloadlen = strlen(buf)+1;
     /*int rc = */client.publish(topic, message);
}

// Reception d'un ordre de manipulation
void messageManipArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  sprintf(buf, "Manipulation %s\n", (char*)message.payload);
  Serial1.print(buf);//affiche le message sur le port série
  char* msg = (char*)message.payload; //stocke le message
  const char* delim = {":"};
  // coupe le message en deux: l'appareil et l'action à faire
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

// Fonction de connexion au broker MQTT
void connect()
{
  sprintf(buf, "Connexion à %s:%d\n", hostname, port);
  Serial1.print(buf);
  int rc = ipstack.connect(hostname, port); // tentative de connexion
  afficherRcAnormal("TCP connect rc: ", rc, 1);
 
  Serial1.println("Connexion a MQTT");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"Terrarium1";
  rc = client.connect(data);
  afficherRcAnormal("MQTT connect rc: ", rc, 0);
  Serial1.println("Connecte a MQTT");  

  rc = client.subscribe(topicManipulations, MQTT::QOS1, messageManipArrived);
  afficherRcAnormal("MQTT manipulation subscribe rc: ", rc, 0);
  Serial1.print("Souscrit a ");
  Serial1.println(topicManipulations);
}

/**************************
 * La fonction setup d'Arduino s'effectue une seule fois après la remise à zéro
 **************************/
void setup()
{
  Serial1.begin(9600);

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
  pompe (AUTO);
  lampe (/*AUTO*/);
}

/**************************
 * La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
 **************************/
void loop()
{
  sensors_event_t event;
  
  // Lumière
  tsl.getEvent(&event);
  if ((event.light == 0.0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
/* If event.light = 0 lux the sensor is probably saturated
 * and no reliable data could be generated!
 * if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.println("Erreur luminosite");
  }
  else
  {
    pushData(event.light, topicLuminosite); // envoie la valeur de luminosité sur le topic correspondant
  }

  // Température
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println("Erreur temperature");
  }
  else
  {
    pushData(event.temperature, topicTemperature);
    reguleTemp(event.temperature);
  }

  // Humidite
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity) || event.relative_humidity == 0.0)
  {
    Serial.println("Erreur humidite");
  }
  else
  {
    pushData(event.relative_humidity, topicHumidite);
    reguleHum(event.relative_humidity);
  }
  lampe(/*AUTO*/);
  pompe (AUTO);
  delay(600000); // attente de 10 minutes avant de recommencer la boucle
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte
}
