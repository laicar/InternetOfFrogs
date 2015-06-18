# Internet of Frogs #
Home automation arduino programs and website for a tropical terrarium using the MQTT messaging protocol.

Système de domotique appliqué à un terrarium tropical communiquant via le protocole de messagerie MQTT. Basé sur [le projet domotique du Fab Lab Provence](http://wiki.labaixbidouille.com/index.php?title=Domotique).

**Matériel utilisé pour la partie électronique**
 - Raspberry Pi 
 - 1 ou 2 Arduinos selon la version du système utilisée
 - Le nombre d'Arduinos de shields Ethernet
 - Le même nombre plus un câbles Ethernet
 - Capteur Adafruit TSL2591 (luminosité)
 - Capteur Adafruit DHT22 (température et humidité)
 - Modules relais pour 6 appareils
 - De quoi alimenter tous les modules et capteurs s'ils sont nombreux (selon carte et matériel utilisés)

**Librairies Arduino utilisées**
 - [Arduino MQTT Client](https://projects.eclipse.org/projects/technology.paho/downloads)
 - [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_DHT_Unified)
 - [Adafruit TSL2591 Library](https://github.com/adafruit/Adafruit_TSL2591_Library)
 - [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
 - [Adafruit DHT Unified](https://github.com/adafruit/Adafruit_DHT_Unified)

**A propos des différentes versions**
Les programmes sont découpables en parties "capteur" pour récolter des données, "relais" pour allumer ou éteindre les bonnes machines et "communication MQTT" pour les échanges de messages.
 - La version locale ne fait pas communiquer les arduinos, ils ne font qu'afficher sur le port série. C'est une version basique inexploitable en l'état.
 - La version 2 arduinos fait communiquer les deux arduinos par Ethernet avec le protocole MQTT. Il n'y a pas 