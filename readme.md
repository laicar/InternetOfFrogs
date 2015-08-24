# Internet of Frogs #
Home automation arduino programs and website for a tropical terrarium using the MQTT messaging protocol.

Système de domotique appliqué à un terrarium tropical communiquant via le protocole de messagerie MQTT. Basé sur [le projet domotique du Fab Lab Provence](http://wiki.labaixbidouille.com/index.php?title=Domotique).

**Matériel utilisé pour la partie électronique**
 - 1 Raspberry Pi 
 - 1 ou 2 Arduinos. Attention, les Uno n'arrive pas à communiquer par Ethernet et à capter via I2C en même temps.
 - Le nombre d'Arduinos en shields Ethernet
 - Le même nombre plus un câbles Ethernet
 - Un switch ou hub (une box internet)
 - Capteur Adafruit TSL2591 (luminosité)
 - Capteur Adafruit DHT22 (température et humidité)
 - Modules relais pour 5 appareils
 - Selon carte et matériel utilisés, de quoi démultiplier les alimentations, sorties SDA/SCL et les masses.

**Librairies Arduino utilisées**
 - [Arduino MQTT Client](https://projects.eclipse.org/projects/technology.paho/downloads)
 - [Adafruit Unified Sensor Library](https://github.com/adafruit/Adafruit_DHT_Unified)
 - [Adafruit TSL2591 Library](https://github.com/adafruit/Adafruit_TSL2591_Library)
 - [Adafruit TSL2561 Unified Library] (https://github.com/adafruit/Adafruit_TSL2561)
 - [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
 - [Adafruit DHT Unified Library](https://github.com/adafruit/Adafruit_DHT_Unified)
