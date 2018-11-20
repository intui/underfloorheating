# underfloorheating
# smartes Thermostat Fußbodenheizung

English below.

Showcase Projekt zur Erprobung des Zusammenspiels von ioT, Cloud und Business Intelligence

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

We built a micro controller based IoT thermostat device that measures temperature and transfers the values to a back-end. The back end analyses these values and computes an individual program with instructions that is sent back to the device.
- Usability: A smart thermostat must allow the user to set target temperatures remotely. A timetable for different heating programs would be nice.
- Energy efficiency: traditional thermostats have one big problem: The reaction time of the heating is very slow. Typically it takes 3-5 hours for the temperature to rise to a newly selected target value.Also it takes a very long time of a room to cool down again before the thermostat switches on again and the process starts from the beginning. This results in a continuous and slow rise and fall of the temperature around the desired value.This is where the main focus of our project lies. To create new approaches and algorithms to "flatten" the temperature curve and thereby keep the room temperature as close as possible to the desired value for a certain time span.
