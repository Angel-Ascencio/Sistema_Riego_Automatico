# Sistema_Riego_Automatico

Codigo para el circuito de un sistema de riego automatico y a la vez manual, con informacion de luminicidad recibida, tanque de agua, humedad de la tierra y del ambiente para el monitoreo de las plantas o del jardir. El proyecto cuenta con version de Arduino y para ESP32.

* Placa (ESP32 o Arduino)
* DHT11
* HC-SR04
* LDR (fotorresistor)
* Resistencia 10KΩ (para LDR)
* Sensor humedad suelo YL-69
* Módulo relé (en este caso se uso un módulo relé de 8 modulos)
* Mini bomba de agua sumergible DC 3v o 5v
* Tubo de silicona para bomba
* Protoboard
* cables jumper
* Fuente de alimentación (eliminador de 9v)
* Recipiente para agua

## Realizado con:
* Arduino IDE 2.3.6

## Instalar los controladores (drivers) para el ESP32 para chips USB-a-Serial.
* CP210x_Universal_Windows_Driver
