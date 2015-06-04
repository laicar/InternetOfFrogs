/**************************
 * Capteur.ino
 * Programme de lecture et d'envoi par IP selon le protocole MQTT
 * des valeurs captées de lumière, de température et d'humidité
 * Basé sur les programmes du wiki LABAixBidouille
 * Contributeurs: Nicolas MULLER, Guy SINNIG, Carole LAI TONG
 **************************/
 
/**************************
 * Fichiers des librairies utilisées
 **************************/
#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <DHT.h>
#include <DHT_U.h>

/**************************
 * Déclaration et initialisation des variables
 **************************/
// Avec #define, toute instance du premier mot sera remplacée par le deuxième dans le code
#define DHTPIN 2 // la borne data du capteur est branchée sur la broche 2
#define DHTTYPE DHT22 // on utilise un capteur humidité/température DHT22

DHT_Unified dht(DHTPIN, DHTTYPE); // on indique la broche et le type de capteur température/humidité
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // on indique le type de capteur de luminosité

char printbuf[100]; //
char buf[200]; // Buffer for sending and receiving Quality of Service messages
EthernetClient c;
IPStack ipstack (c);
MQTT::Message message; // crée un message MQTT
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

/**************************
 * Configuration MQTT
 **************************/
char hostname[] = "192.168.0.9";// Adresse IP du broker
int port = 1883;// Port utilisé par le broker

byte mac[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }; // Adresse MAC de l'arduino
byte ip[] = { 192,168,0,28 }; // Adresse IP arduino

// Noms des topics où on publie des messages
const char* topicTemperature = "InternetOfFrogs/Terrarium1/Valeur/Temperature";
const char* topicHumidite = "InternetOfFrogs/Terrarium1/Valeur/Humidite";
const char* topicLuminosite = "InternetOfFrogs/Terrarium1/Valeur/Luminosite";

/**************************
 * METHODES
 **************************/
// Fonction de connexion au broker MQTT
void connect()
{
  sprintf(printbuf, "Connexion à %s:%d\n", hostname, port);
  Serial.print(printbuf); // affiche qu'on se connecte à l'hôte donné sur le port donné
  int rc = ipstack.connect(hostname, port); // tentative de connexion
  if (rc != 1) // si on n'a pas réussi à se connecter on affiche le numéro d'erreur obtenu
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }
 
  Serial.println("Connexion a MQTT");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"arduino-sample";
  rc = client.connect(data);
  if (rc != 0)
  {  
    sprintf(printbuf, "Le code de retour de la fonction MQTT connect est %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("Connecte a MQTT");  
}

// Envoie une valeur sur le topic adequat
void pushData(float valeur, const char *topic)
{
     sprintf(buf, "%f", valeur); // met la valeur dans un tampon 
     Serial.println(buf); // envoie le contenu du tampon sur le port série
     message.retained = false;
     message.dup = false;
     message.payload = (void*)buf;
     message.qos = MQTT::QOS1;
     message.payloadlen = strlen(buf)+1;
     int rc = client.publish(topic, message);
}

//La fonction setup d'Arduino s'effectue une seule fois après la remise à zéro
void setup()
{
  Serial.begin(9600); // initialisation de la liaison série à 9600 bauds
  Ethernet.begin(mac,ip);
  Serial.println("MQTT Internet of Frogs");
  connect(); // connexion au broker
  dht.begin(); // mise en route du capteur humidité/température
  // Règlage de la sensibilité à la lumière
  tsl.setGain(TSL2591_GAIN_MED);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
}

// La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
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
  }
  delay(5000); // attente de 5 secondes avant de recommencer la boucle
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte
}