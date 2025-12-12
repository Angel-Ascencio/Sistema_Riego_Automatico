#include <DHT.h>

// DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensor ultrasónico HC-SR04
const int trigPin = 9;
const int echoPin = 10;

// Medidas del contenedor (en cm)
const float largoContenedor = 18.0;
const float anchoContenedor = 13.5;
const float profundidadTotalTanque = 7.0;
const float elevacionSensor = 1.0;

// Sensor LDR
const int ldrPin = A0;
const int resistenciaFija = 10000;

// Sensor de humedad de sustrato
const int sensorHumedadPin = A1;

// Relé para bomba
const int relePin = 8;


bool bombaEncendida = false;
unsigned long tiempoInicioBomba = 0;
const unsigned long duracionBomba = 10000;      // 10 segundos
const unsigned long tiempoEsperaDespues = 20000; // 20 segundos de espera
unsigned long tiempoUltimaActivacion = 0;

int comando = 0;

// Variables para sensor ultrasónico
long duracion;
float distanciaCmSensor;
float distanciaDesdeBordeTanque;
float alturaAguaCm;
float porcentajeLlenado;

void setup() {
  Serial.begin(9600);
  
  // Inicializar DHT
  dht.begin();
  
  // Configurar pines
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relePin, OUTPUT);
  
  // Apagar bomba inicialmente
  digitalWrite(relePin, HIGH);
  
  Serial.println(" SISTEMA DE MONITOREO DE PLANTAS ");
  Serial.println("Envia '1' por Serial para activar la bomba por 10 segundos.");
  Serial.println(" ---------------------------------- ");
}

void loop() {
  unsigned long tiempoActual = millis();
  
  if (Serial.available() > 0) {
    comando = Serial.parseInt();
    
    if (comando == 1 && !bombaEncendida) {
      Serial.println("Bomba activada por 10 segundos (manual)");
      digitalWrite(relePin, LOW); // Encender bomba
      bombaEncendida = true;
      tiempoInicioBomba = tiempoActual;
      tiempoUltimaActivacion = tiempoActual;
    } else if (comando != 1) {
      Serial.println("Comando no reconocido. Usa '1' para activar la bomba.");
    }
    
    while (Serial.available() > 0) Serial.read();
  }
  
  // LECTURA SENSOR HUMEDAD SUSTRATO
  int valorSensor = analogRead(sensorHumedadPin);
  float porcentajeHumedadSustrato = 100 - ((float)valorSensor / 1023.0) * 100.0;
  
  // valores entre 0-100%
  if (porcentajeHumedadSustrato < 0) porcentajeHumedadSustrato = 0;
  if (porcentajeHumedadSustrato > 100) porcentajeHumedadSustrato = 100;
  
  // ACTIVACIÓN AUTOMÁTICA POR HUMEDAD
  if (!bombaEncendida &&
      porcentajeHumedadSustrato < 50.0 &&
      (tiempoActual - tiempoUltimaActivacion >= duracionBomba + tiempoEsperaDespues)) {
    Serial.println("Bomba activada por 10 segundos (humedad baja)");
    digitalWrite(relePin, LOW); // Encender bomba
    bombaEncendida = true;
    tiempoInicioBomba = tiempoActual;
    tiempoUltimaActivacion = tiempoActual;
  }
  
  // APAGADO AUTOMÁTICO DE BOMBA
  if (bombaEncendida && (tiempoActual - tiempoInicioBomba >= duracionBomba)) {
    digitalWrite(relePin, HIGH); // Apagar bomba
    bombaEncendida = false;
    Serial.println("Bomba apagada automaticamente.");
  }
  
  // LECTURA SENSOR DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Verificar si las lecturas son validas
  if (isnan(h) || isnan(t)) {
    Serial.print("Error DHT11\t");
  } else {
    Serial.print("H: ");
    Serial.print(h, 1);
    Serial.print("%\tT: ");
    Serial.print(t, 1);
    Serial.print("°C\t");
  }
  
  // LECTURA SENSOR ULTRASÓNICO 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duracion = pulseIn(echoPin, HIGH, 30000); // Timeout 30ms
  
  if (duracion > 0) {
    distanciaCmSensor = duracion * 0.0343 / 2;
    distanciaDesdeBordeTanque = distanciaCmSensor - elevacionSensor;
    alturaAguaCm = profundidadTotalTanque - distanciaDesdeBordeTanque;
    
    // Limitar valores
    if (alturaAguaCm < 0) alturaAguaCm = 0;
    if (alturaAguaCm > profundidadTotalTanque) alturaAguaCm = profundidadTotalTanque;
    
    porcentajeLlenado = (alturaAguaCm / profundidadTotalTanque) * 100.0;
    
    Serial.print("Agua: ");
    Serial.print(porcentajeLlenado, 1);
    Serial.print("%\t");
  } else {
    Serial.print("Error US\t");
  }
  
  // LECTURA SENSOR LDR 
  int lecturaLDRCruda = analogRead(ldrPin);
  int intensidadLuzPorcentaje = map(lecturaLDRCruda, 0, 1023, 0, 100);
  
  Serial.print("Luz: ");
  Serial.print(intensidadLuzPorcentaje);
  Serial.print("%\t");
  
  // MOSTRAR HUMEDAD SUSTRATO
  Serial.print("Sustrato: ");
  Serial.print(porcentajeHumedadSustrato, 1);
  Serial.print("%\t");
  
  // ESTADO BOMBA
  Serial.print("Bomba: ");
  Serial.print(bombaEncendida ? "ON " : "OFF");
  
  // TIEMPO RESTANTE SI BOMBA ENCENDIDA
  if (bombaEncendida) {
    unsigned long tiempoRestante = duracionBomba - (tiempoActual - tiempoInicioBomba);
    Serial.print(" (");
    Serial.print(tiempoRestante / 1000);
    Serial.print("s)");
  }
  
  Serial.println();
  
  delay(2000);
}