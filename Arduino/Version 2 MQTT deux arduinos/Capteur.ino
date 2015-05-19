/**************************
 * Emetteur.ino
 * Programme de lecture et d'envoi par IP selon le protocole MQTT
 * des valeurs captées de lumière, de température et d'humidité
 * Basé sur les programmes du wiki LABAixBidouille Emetteur MQTT et
 * sur celui des capteurs de lumière, température et humidité
 * Contributeurs: Nicolas MULLER, Guy SINNIG, Carole LAI TONG
 **************************/
 
/**************************
 * Fichiers header des librairies utilisées
 * Déclarent les valeurs et les fonctions qui seront utilisées dans ce programme
 **************************/
#include <SPI.h>
#include <Ethernet.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <DHT.h>

/**************************
 * Déclaration et initialisation des variables
 * Avec #define, toute instance du premier mot
 * sera remplacée par le deuxième dans le code
 **************************/
#define MQTTCLIENT_QOS2 1 // qualité de service MQTT mise au niveau 1
#define DHTPIN 2 // la borne data du capteur est branchée sur la broche 2
#define DHTTYPE DHT11 // on utilise un capteur humidité/température DHT11

DHT dht(DHTPIN, DHTTYPE); // on indique la broche et le type de capteur
float h = 0;  // initialisation de la variable d'humidité
float t = 0;  // initialisation de la variable de température
int l = 0;  // initialisation de la variable d'éclairement
int ldr = A0; // broche sur laquelle est branchée la borne data de la LDR (le capteur de lumière)

//int compteur = 0;
char printbuf[100]; //
char buf[200]; // Buffer for sending and receiving Quality of Service messages
EthernetClient c;
IPStack ipstack (c);
MQTT::Message message; // crée un message MQTT (pour l'instant vide)
// crée un client MQTT
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

/**************************
 * Configuration MQTT
 **************************/

// Adresse IP du broker
char hostname[] = "192.168.0.9";

// Port utilisé par le broker
int port = 1883;

// Noms des topics où on publie des messages
const char* topicTemperature = "InternetOfFrogs/temperature";
const char* topicHumidite = "InternetOfFrogs/humidite";
const char* topicLuminosite = "InternetOfFrogs/luminosite";

// à remplacer avec l'adresse MAC de l'arduino émetteur
byte mac[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };  
byte ip[] = { 192,168,0,28 }; // adresse IP arduino émetteur

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
void pushData(int valeur, const char *topic)
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

/**************************
 * La fonction setup d'Arduino s'effectue une seule fois après la remise à zéro
 **************************/
void setup()
{
  Serial.begin(9600); // initialisation de la liaison série à 9600 bauds
  Ethernet.begin(mac,ip);
  Serial.println("MQTT Internet of Frogs");
  connect(); // connexion au broker
  dht.begin(); // mise en route (?) du capteur humidité/température
}

/**************************
 * La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
 **************************/
void loop()
{
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte
/**************************
 * Mesure
 **************************/
  l = analogRead(ldr); // mesure de la lumière reçue par la LDR
  // convertion de la valeur lue en pourcentage de luminosité captable par ce capteur
  l = map (l,0,1023,100,0);
  h = dht.readHumidity(); // mesure de l'humidité
  t = dht.readTemperature(); // mesure de la température
/**************************
 * Affichage
 **************************/
  if (isnan(l))
  {
    Serial.print( "Lecture de la luminosite impossible \t");
  // message d'erreur affiché sur le port série au cas où
  //la LDR n'arrive pas à lire correctement et ne renvoie pas un nombre
  }
  else
  {
    pushData(l, topicLuminosite); // envoie la valeur de luminosité avec le bon topic
  }
  if (isnan(t) || t == 0.00)
  {
    Serial.print( "Lecture de la temperature impossible \t"); // test d'erreur
  }
  else
  {
    pushData(t, topicTemperature);
  }
  if (isnan(h) || h == 0.00)
  {
    Serial.println( "Lecture de l'humidite impossible"); // test d'erreur
  }
  else
  {
    pushData(h, topicHumidite);
  }
  delay(1000); // attente de 1 seconde avant de recommencer la boucle
}
