# Internet of Frogs #
Home automation arduino programs and website for a tropical terrarium using the MQTT messaging protocol.

Système de domotique appliqué à un terrarium tropical communiquant via le protocole de messagerie MQTT. Basé sur [le projet domotique du Fab Lab Provence](http://wiki.labaixbidouille.com/index.php?title=Domotique).

**Matériel utilisé pour la partie électronique**
 - Raspberry Pi 
 - 2 Arduinos Uno ou 1 Arduino Mega selon la version du système utilisée
 - Capteur Adafruit TSL2591 (luminosité)
 - Capteur Adafruit DHT22 (température et humidité)
 - Module relais

**Librairies Arduino utilisées**
 - [Arduino MQTT Client](https://projects.eclipse.org/projects/technology.paho/downloads)
 - [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_DHT_Unified)
 - [Adafruit TSL2591 Library](https://github.com/adafruit/Adafruit_TSL2591_Library)
 - [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
 - [Adafruit DHT Unified](https://github.com/adafruit/Adafruit_DHT_Unified)