/*
* Relais.ino Version sans communication
* Programme pour allumer et éteindre des appareil électriques
* avec un ou plusieurs module(s) double relais Arduino
* et une horloge rtc pour savoir s'il fait jour ou non
*/

#include <Wire.h>
#include <RTClib.h>

// Bornes sur lesquelles les cables du relais
// correspondant à chaque appareil seront branchés
#define CHAUFF 8
#define FOGGER 9
#define VENT 10
#define LUMIERE 11

#define H_MATIN 6
#define H_SOIR 20

#define ALLUMER 0
#define ETEINDRE 1

RTC_DS1307 rtc;
bool chaufOn = false;
bool fogOn = true;
bool pompeOn = false;
bool ventOn = true;

void chauffage (int action)
{
  if (action == ALLUMER && !chaufOn)
  {
    digitalWrite(CHAUFF,LOW);
    chaufOn = true;
    Serial.println("Allume le chauffage");
  }
  else if (action == ETEINDRE && chaufOn)
  {
    digitalWrite(CHAUFF,HIGH);
    chaufOn = false;
    Serial.println("Eteint le chauffage");
  }
}

void refroidissement (int action)
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

void ventilation (int action)
{
  if (action == ALLUMER && !ventOn)
  {
    digitalWrite(VENT,LOW);
    ventOn = true;
    Serial.println("Allume la ventilation");
  }
  else if (action == ETEINDRE && ventOn)
  {
    digitalWrite(VENT,HIGH);
    ventOn = false;
    Serial.println("Eteint la ventilation");
  }
}

void humidificateur (int action)
{
  if (action == ALLUMER && !fogOn)
  {
    digitalWrite(FOGGER,LOW);
    fogOn = true;
    Serial.println("Allume le mist fogger");
  }
  else if (action == ETEINDRE && fogOn)
  {
    digitalWrite(FOGGER,HIGH);
    fogOn = false;
    Serial.println("Eteint le mist fogger");
  }
}

void lampe ()
{
  if (faitJour()) digitalWrite(LUMIERE,LOW);
  else digitalWrite(LUMIERE,HIGH);
  
}

bool faitJour ()
{
  DateTime now = rtc.now();
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

void setup()
{
  Serial.begin(9600);
  pinMode(CHAUFF, OUTPUT);
  pinMode(FOGGER, OUTPUT);
  pinMode(VENT, OUTPUT);
  pinMode(LUMIERE, OUTPUT);
  rtc.begin();
}

  void loop()
{
  // TODO: lecture/reception valeurs captées...
  float hum = 42.42;
  float temp = 10.25;

  reguleTemp(temp);
  reguleHum(hum);
  lampe();
  delay(60000); // attend une minute
}
