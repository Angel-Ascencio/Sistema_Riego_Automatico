#include <DHT.h>

//  DHT11
#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//  Sensor ultrasonico 
const int trigPin = 27;
const int echoPin = 26;

// Medidas del contenedor
const float largoContenedor = 18.0;
const float anchoContenedor = 13.5;
const float profundidadTotalTanque = 7.0;
const float elevacionSensor = 1.0;

long duracion;
float distanciaCmSensor;
float distanciaDesdeBordeTanque;
float alturaAguaCm;
float porcentajeLlenado;

//  Sensor LDR 
const int ldrPin = 36;
const int resistenciaFija = 10000;

//  Sensor de humedad de sustrato 
const int sensorHumedadPin = 34;

//  Rele para bomba 
const int relePin = 25;
bool bombaEncendida = false;
unsigned long tiempoInicioBomba = 0;
const unsigned long duracionBomba = 10000;      // 10 segundos
const unsigned long tiempoEsperaDespues = 20000; // 20 segundos de espera despues de apagar
unsigned long tiempoUltimaActivacion = 0;

int comando = 0;

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  analogReadResolution(12); // ADC 0-4095

  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH); // Bomba apagada al inicio

  Serial.println("Sistema listo. Envia '1' por Serial para activar la bomba por 10 segundos.");
}

void loop() {
  unsigned long tiempoActual = millis();

  //  LECTURA DE COMANDO SERIAL 
  if (Serial.available() > 0) {
    comando = Serial.parseInt();

    if (comando == 1 && !bombaEncendida) {
      Serial.println("Bomba activada por 10 segundos (manual)...");
      digitalWrite(relePin, LOW); // Encender bomba
      bombaEncendida = true;
      tiempoInicioBomba = tiempoActual;
      tiempoUltimaActivacion = tiempoActual;
    } else {
      Serial.println("Comando no reconocido. Usa '1' para activar la bomba.");
    }

    while (Serial.available() > 0) Serial.read(); // Limpiar buffer
  }

  //  LECTURA HUMEDAD SUSTRATO
  int valorSensor = analogRead(sensorHumedadPin);
  float porcentajeHumedadSustrato = 100 - ((float)valorSensor / 4095.0) * 100.0;
  porcentajeHumedadSustrato = constrain(porcentajeHumedadSustrato, 0, 100);

  Serial.print("Humedad sustrato: ");
  Serial.print(porcentajeHumedadSustrato, 2);
  Serial.print("%\t");

  //  ACTIVACION AUTOMÁTICA POR HUMEDAD 
  if (!bombaEncendida
      && porcentajeHumedadSustrato < 50.0
      && (tiempoActual - tiempoUltimaActivacion >= duracionBomba + tiempoEsperaDespues)) {
    Serial.println("Bomba activada por 10 segundos (automatico, humedad baja)...");
    digitalWrite(relePin, LOW); // Encender bomba
    bombaEncendida = true;
    tiempoInicioBomba = tiempoActual;
    tiempoUltimaActivacion = tiempoActual;
  }

  //  GESTION TEMPORIZADA DE LA BOMBA 
  if (bombaEncendida && (tiempoActual - tiempoInicioBomba >= duracionBomba)) {
    digitalWrite(relePin, HIGH); // Apagar bomba
    bombaEncendida = false;
    Serial.println("Bomba desactivada automáticamente despues de 10 segundos.");
  }

  //  LECTURAS DE OTROS SENSORES 
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print("Humedad ambiente: ");
  Serial.print(h);
  Serial.print(" %\t");

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" °C\t");

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracion = pulseIn(echoPin, HIGH);
  distanciaCmSensor = duracion * 0.0343 / 2;
  distanciaDesdeBordeTanque = distanciaCmSensor - elevacionSensor;
  alturaAguaCm = profundidadTotalTanque - distanciaDesdeBordeTanque;

  alturaAguaCm = constrain(alturaAguaCm, 0, profundidadTotalTanque);
  porcentajeLlenado = (alturaAguaCm / profundidadTotalTanque) * 100;

  Serial.print("Nivel de agua: ");
  Serial.print(porcentajeLlenado, 2);
  Serial.print("%\t");

  int lecturaLDRCruda = analogRead(ldrPin);
  int intensidadLuzPorcentaje = map(lecturaLDRCruda, 0, 4095, 0, 100);

  Serial.print("Luz: ");
  Serial.print(intensidadLuzPorcentaje);
  Serial.println("%");

  delay(1000);
}
