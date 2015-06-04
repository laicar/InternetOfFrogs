/**************************
 * MegaArduino.ino
 * Programme utilisant capteurs et relais pour maintenir des conditions tropicales
 * Utilise le protocole MQTT pour envoyer les valeurs captées et recevoir des commandes via IP
 * Contributeurs: Nicolas MULLER, Guy SINNIG, Carole LAI TONG
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
#define DHTPIN 2 // la borne data du capteur temp/hum est branchée sur la broche 2
#define DHTTYPE DHT22 // on utilise un capteur humidité/température DHT22

DHT_Unified dht(DHTPIN, DHTTYPE); // on indique la broche et le type de capteur température/humidité
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // on indique le type de capteur de luminosité

// Variables Relais
// Bornes sur lesquelles les cables du relais
// correspondant à chaque appareil seront branchés
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

// Variables MQTT
char hostname[] = "192.168.0.9";// Adresse IP du broker
int port = 1883;// Port utilisé par le broker

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4E, 0xB0 }; // Adresse MAC de l'arduino émetteur
byte ip[] = { 192,168,0,28 }; // adresse IP arduino

// Noms des topics où on publie ou lit des messages
const char* topicTemperature = "InternetOfFrogs/Terrarium1/Valeur/Temperature";
const char* topicHumidite = "InternetOfFrogs/Terrarium1/Valeur/Humidite";
const char* topicLuminosite = "InternetOfFrogs/Terrarium1/Valeur/Luminosite";
const char* topicManipulations = "InternetOfFrogs/Terrarium1/Manipulation";

char printbuf[100]; //Tampon pour envoyer et recevoir des messages
char buf[200]; // Tampon pour recevoir et envoyer des messages de Quality of Service
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

void lampe()
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
    Serial1.println("Allume le chauffage");
  }
  else if (action == ETEINDRE && chaufOn)
  {
    digitalWrite(CHAUFF,HIGH);
    chaufOn = false;
    Serial1.println("Eteint le chauffage");
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
    Serial1.println("Allume la ventilation");
  }
  else if (action == ETEINDRE && ventOn)
  {
    digitalWrite(VENT,HIGH);
    ventOn = false;
    Serial1.println("Eteint la ventilation");
  }
}

void humidificateur (const int action)
{
  if (action == ALLUMER && !fogOn)
  {
    digitalWrite(FOGGER,LOW);
    fogOn = true;
    Serial1.println("Allume le mist fogger");
  }
  else if (action == ETEINDRE && fogOn)
  {
    digitalWrite(FOGGER,HIGH);
    fogOn = false;
    Serial1.println("Eteint le mist fogger");
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

// Methodes pour la communication MQTT

void afficherRcAnormal(char* msg, int rc, int successRc)
{
  if (rc != successRc)
  Serial1.print(msg);
  Serial1.println(rc);
}

// Envoie une valeur sur le topic adequat
void pushData(int valeur, const char *topic)
{
     sprintf(buf, "%f", valeur); // met la valeur dans le tampon buf
     Serial1.println(buf); // envoie le contenu du tampon buf sur le port série
     message.retained = false;
     message.dup = false;
     message.payload = (void*)buf;
     message.qos = MQTT::QOS1;
     message.payloadlen = strlen(buf)+1;
     int rc = client.publish(topic, message);
}

// Reception d'un ordre de manipulation
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

// Fonction de connexion au broker MQTT
void connect()
{
  sprintf(printbuf, "Connexion à %s:%d\n", hostname, port);
  Serial1.print(printbuf); // affiche qu'on se connecte à l'hôte donné sur le port donné
  int rc = ipstack.connect(hostname, port); // tentative de connexion
  afficherRcAnormal("TCP connect rc: ", rc, 1);
 
  Serial1.println("Connexion a MQTT");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"arduino-mega";
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
  Serial1.begin(9600); // initialisation de la liaison série pour communiquer à 9600 bauds
  Serial2.begin(115200); // initialisation de la liaison série pour capteurs à 115200 bauds

  Ethernet.begin(mac,ip);
  connect(); // connexion au broker

  pinMode(CHAUFF, OUTPUT); // on va envoyer des ordres sur telle et telle broche
  pinMode(FOGGER, OUTPUT);
  pinMode(VENT, OUTPUT);
  pinMode(LUMIERE, OUTPUT);

  rtc.begin(); // mise en route de l'horloge
  dht.begin(); // mise en route du capteur humidité/température
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
  lampe();
  delay(60000); // attente de 1 minute avant de recommencer la boucle
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte
}
