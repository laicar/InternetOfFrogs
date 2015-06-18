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

  float temp = 41.3;
  float hum = 61.2;

  reguleTemp(temp);
  reguleHum(hum);
  lampe();
  delay(60000); // attend une minute
}
