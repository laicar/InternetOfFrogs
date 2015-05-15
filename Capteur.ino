/**************************
 * Programme de lecture et d'affichage de la lumière,
 * de la température et de l'humidité via deux capteurs
 **************************/
#include <DHT.h>

/**************************
 * Déclaration et initialisation des variables
 * avec #define, toute instance du premier mot
 * sera remplacée par le deuxième dans le code
 **************************/
#define DHTPIN 2 // la borne data du capteur est branchée sur la broche 2
#define DHTTYPE DHT22 // on utilise un capteur humidité/température DHT11

DHT dht(DHTPIN, DHTTYPE); // on indique la broche et le type de capteur
float h = 0;  // initialisation de la variable d'humidité
float t = 0;  // initialisation de la variable de température
int mes = 0;  // initialisation de la variable d'éclairement
int ldr = A0; // broche sur laquelle est branchée la borne data de la LDR (le capteur de lumière)

/**************************
 * La fonction setup d'Arduino est déroulée une seule fois après la remise à zéro
 **************************/
void setup()
{
  Serial.begin(115200); // initialisation de la liaison série à 115200 bauds
  dht.begin(); // initialisation du capteur humidité/température
}

/**************************
 * La fonction loop d'Arduino s'effectue en boucle indéfiniment après setup
 **************************/
void loop()
{
/**************************
 * Mesure
 **************************/
  mes = analogRead(ldr); // mesure de la lumière reçue par la LDR via sa résistance
  mes = map (mes,0,1023,100,0);
  // convertion de la résistance lue en pourcentage de luminosité (captable par ce capteur)
  h = dht.readHumidity(); // mesure de l'humidité
  t = dht.readTemperature(); // mesure de la température
/**************************
 * Affichage
 **************************/
  if (isnan(mes))
    Serial.print( "Lecture de la luminosite impossible \t");
  // message d'erreur au cas où la LDR n'arrive pas à lire correctement et ne renvoie pas un nombre
  else {
    Serial.print ("Luminosite: ");
    Serial.print (mes); // affiche valeur luminosité mesuree et mappee
    Serial.print (" % \t");
  }
  if (isnan(t) || t == 0.00)
    Serial.print( "Lecture de la temperature impossible \t"); // test d'erreur
  else {
    Serial.print("Temperature: ");
    Serial.print(t); // affichage valeur température
    Serial.print(" *C\t");
  }
  if (isnan(h) || h == 0.00)
    Serial.println( "Lecture de l'humidite impossible"); // test d'erreur
  else {
   Serial.print("Humidite: ");
   Serial.print(h); // affichage valeur humidité
   Serial.println(" %");
  }
  delay(1000); // attente de 1 seconde
}
