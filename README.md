# underfloorheating
# smartes Thermostat Fußbodenheizung

English below.

SHOWCASE PROJEKT ZUR ERPROBUNG DES ZUSAMMENSPIELS VON IOT, CLOUD UND BUSINESS INTELLIGENCE

Der Plan
1.	IoT device ersetzt herkömmliches Thermostat zur Steuerung der Fußbodenheizung
2.	Sensordaten werden an eine Anwendung in der Cloud geschickt und verarbeitet
3.	Steuerungsfunktion übermittelt Anweisungen an iot-Thermostat

Die Komponenten
1.	IoT device:
Bestandteile: Microcontroller, Klima-Sensor, Display, Potentiometer, Relais, Netzteil.
Der Microcontroller kommuniziert mit dem Backend um:
Das Gerät am Service anzumelden und zu authentifizieren,
Instruktionen vom Service abzuholen,
Messdaten an den Service zu senden.
2.	Web Service Backend:
Stellt die notwendigen Schnittstellen bereit um:
Iot Geräte zu verwalten und zu verifizieren,
Instruktionen wie z.B. ein geändertes Heizprogramm an das ioT device zu senden,
Sensordaten zu empfangen um diese dann weiter zu verarbeiten – beispielsweise charakteristische Kennzahlen des Raumes zu erheben.
3.	Statistik-Engine:
Führt die Auswertung von Sensordaten durch mit dem Ziel:
Relevante Kennzahlen des Raumes und der Wohnung ermitteln, die vom Heizprogramm des Thermostats verwendet werden können.


EN:
Using the cloud and ioT to create a connected smart heating system for underfloor heating systems.

More details coming soon.
