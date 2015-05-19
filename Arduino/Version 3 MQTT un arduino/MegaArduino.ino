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
#include <DHT.h> // Capteurs humidité/température
#include <Wire.h>
#include <RTClib.h> // Horloge

/**************************
 * Déclaration et initialisation des variables
 * Avec #define, toute instance du premier mot
 * sera remplacée par le deuxième dans le code
 **************************/

// Variables Capteurs
#define DHTPIN 2 // la borne data du capteur temp/hum est branchée sur la broche 2
#define DHTTYPE DHT22 // on utilise un capteur humidité/température DHT22

DHT dht(DHTPIN, DHTTYPE); // on indique la broche et le type de capteur
float h = 0;  // initialisation de la variable d'humidité
float t = 0;  // initialisation de la variable de température
int l = 0;  // initialisation de la variable d'éclairement
int ldr = A0; // broche sur laquelle est branchée la borne data de la LDR (le capteur de lumière)

// Variables Relais
// Bornes sur lesquelles les cables du relais
// correspondant à chaque appareil seront branchés
#define CHAUFF 8
#define FOGGER 9
#define POMPE 10
#define VENT 11
#define LUMIERE 12
//#define NOURRITURE 13

#define H_MATIN 7
#define H_SOIR 19

#define ALLUMER 0
#define ETEINDRE 1
#define AUTO 2

RTC_DS1307 rtc;
bool chaufOn = false;
bool fogOn = true;
bool pompeOn = false;
bool ventOn = true;
bool lumAuto = true;
uint32_t heureMiseLumiereManuelle;

// Variables MQTT
char hostname[] = "192.168.0.9";// Adresse IP du broker
int port = 1883;// Port utilisé par le broker

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x4E, 0xB0 }; // Adresse MAC de l'arduino émetteur
byte ip[] = { 192,168,0,28 }; // adresse IP arduino

// Noms des topics où on publie ou lit des messages
const char* topicTemperature = "InternetOfFrogs/Terrarium1/temperature";
const char* topicHumidite = "InternetOfFrogs/Terrarium1/humidite";
const char* topicLuminosite = "InternetOfFrogs/Terrarium1/luminosite";
const char* topicManipulations = "InternetOfFrogs/Terrarium1/manipulation";

char* manip; // ordre reçu (ex: allumer le chauffage)
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

void pompe (const int action)
{
  if (action == ALLUMER && !pompeOn)
  {
    digitalWrite(POMPE,LOW);
    pompeOn = true;
    Serial1.println("Allume la pompe a eau");
  }
  else if (action == ETEINDRE && pompeOn)
  {
    digitalWrite(POMPE,HIGH);
    pompeOn = false;
    Serial1.println("Eteint la pompe a eau");
  }
}

void lampe (const int action)
{
  if (action == AUTO)
  {
    if (lumAuto)
    {
      if (faitJour()) digitalWrite(LUMIERE,LOW);
      else digitalWrite(LUMIERE,HIGH);
    }
    else
    {// La lumière est bloquée en mode manuel une minute
      if (rtc.now().unixtime() > (heureMiseLumiereManuelle + 60)) 
      {
        lumAuto = true;
        Serial1.println("La lumiere est revenue en mode automatique");
      }
    }
  }
  else if (action == ALLUMER)
  {
    heureMiseLumiereManuelle = rtc.now().unixtime();
    lumAuto = false;
    digitalWrite(LUMIERE,LOW);
    Serial1.println("Allume la lumiere manuellement");
  }
  else if (action == ETEINDRE)
  {
    heureMiseLumiereManuelle = rtc.now().unixtime();
    lumAuto = false;
    digitalWrite(LUMIERE,HIGH);
    Serial1.println("Eteint la lumiere manuellement");
  }
}

bool faitJour ()
{
  DateTime now = rtc.now(); // Récupère la date et l'heure actuelles
  return (now.hour() > H_MATIN && now.hour() < H_SOIR);
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

/**************************
 * Methodes pour la communication MQTT
 **************************/

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

// Reçoit un ordre de manipulation à faire
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
    case POMPE:
      pompe(action);
      break;
    case LUMIERE:
      lampe(action);
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
  data.clientID.cstring = (char*)"arduino-sample";
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
  pinMode(POMPE, OUTPUT);
  pinMode(VENT, OUTPUT);
  pinMode(LUMIERE, OUTPUT);
  pompe (ALLUMER); // on allume la cascade et on n'y touche plus

  rtc.begin(); // mise en route de l'horloge
  dht.begin(); // mise en route du capteur humidité/température
}

/**************************
 * La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
 **************************/
void loop()
{
  if (!client.isConnected()) connect(); // si le client est déconnecté on le reconnecte

  // Mesure
  l = analogRead(ldr); // mesure de la lumière reçue par la LDR
  // convertion de la valeur lue en pourcentage de luminosité captable par ce capteur:
  l = map (l,0,1023,100,0); // plus il y a de lumière plus la valeur sera proche de 100
  h = dht.readHumidity(); // mesure de l'humidité
  t = dht.readTemperature(); // mesure de la température

  // Envoi des mesures sur le web
  // Teste si le capteur n'arrive pas à lire correctement et ne renvoie pas un nombre
  if (isnan(l))
  {
    Serial1.println("Erreur luminosite");
  }
  else
  {
    pushData(l, topicLuminosite); // envoie la valeur de luminosité avec le bon topic
  }
  if (isnan(t) || t == 0.00)
  {
    Serial1.println("Erreur temperature");
  }
  else
  {
    pushData(t, topicTemperature);
  }
  if (isnan(h) || h == 0.00)
  {
    Serial1.println("Erreur humidite");
  }
  else
  {
    pushData(h, topicHumidite);
  }

  // Régulation du terrarium (si nécessaire)
  reguleTemp(t);
  reguleHum(h);
  lampe(AUTO);
  delay(60000); // attente de 1 minute avant de recommencer la boucle
}
