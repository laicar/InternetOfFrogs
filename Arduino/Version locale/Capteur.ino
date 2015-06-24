/**************************
 * Programme de lecture et d'affichage de la lumière,
 * de la température et de l'humidité via deux capteurs
 **************************/

#include <Wire.h>
#include <Adafruit_Sensor.h> // Capteurs Adafruit unifiés
#include <Adafruit_TSL2591.h> // Capteur de lumière TSL2591
#include <DHT.h> // Capteur humidité/température
#include <DHT_U.h> // Capteur humidité/température unifié

/**************************
 * Déclaration et initialisation des variables
 * avec #define, toute instance du premier mot
 * sera remplacée par le deuxième dans le code
 **************************/

// On utilise un capteur humidité/température DHT22
// dont la borne data est branchée sur la broche 2
DHT_Unified dht(2, DHT22);
// On indique le type de capteur de luminosité
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

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
  sensors_event_t event;

  // Lumière
  tsl.getEvent(&event);
  if ((event.light == 0.0) ||
      (event.light > 4294966000.0) ||
      (event.light < -4294966000.0))
  {
/* If event.light = 0 lux the sensor is probably saturated
 * and no reliable data could be generated!
 * if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.print( "Lecture de la luminosite impossible \t");
  }
  else {
    Serial.print ("Luminosite: ");
    Serial.print (event.light);
    Serial.print (" lux\t");
  }
  
  // Température
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.print( "Lecture de la temperature impossible \t");
  }
  else {
    Serial.print ("Temperature: ");
    Serial.print (event.temperature);
    Serial.print (" *C\t");
  }

  // Humidite
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println ( "Lecture de l'humidite impossible");
  }
  else {
   Serial.print ("Humidite: ");
   Serial.print (event.relative_humidity);
   Serial.println (" %");
  }
  delay(1000); // attente de 1 seconde
}
