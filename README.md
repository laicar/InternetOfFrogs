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
 - Une horloge RTC Adafruit DS1307 par Arduino capteur
 - De quoi alimenter tous les modules et capteurs s'ils sont nombreux et de quoi brancher plusieurs composants sur les broches SCL et SDA (selon carte et matériel utilisés)

**Librairies Arduino utilisées**
 - [Arduino MQTT Client](https://projects.eclipse.org/projects/technology.paho/downloads)
 - [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_DHT_Unified)
 - [Adafruit TSL2591 Library](https://github.com/adafruit/Adafruit_TSL2591_Library)
 - [Adafruit DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
 - [Adafruit DHT Unified](https://github.com/adafruit/Adafruit_DHT_Unified)

**A propos des différentes versions**
Les programmes sont découpables en parties "capteur" pour récolter des données, "relais" pour allumer ou éteindre les bonnes machines et "communication MQTT" pour les échanges de messages.
 - La version locale ne fait pas communiquer les arduinos, ils ne font qu'afficher sur le port série. Ce sont des briques de code et ne constituent pas un système fonctionnel.
 - La version à deux arduinos fait communiquer les deux arduinos (capteur et relais) par Ethernet avec le protocole MQTT. Il y a aussi écriture sur le port série.
 - Les versions à un arduino font à la fois capteur et relais sur une même carte. Il faut prévoir combien de composants seront branchés sur l'Arduino et voir le nombre de broches SCL et SDA pour choisir la carte appropriée. La version compacte n'a pas d'écriture sur le port série et peut être utilisée sur par exemple un Arduino Uno. La version basique écrit des messages et les erreurs rencontrées sur le port série mais est plus volumineuse et nécessite par exemple un Arduino Mega.