#include <SoftwareSerial.h>
#include <Servo.h>

// RX, TX Modulo Bluetooth
SoftwareSerial BT(2, 3);

//Leds Cuartos
int ledCocina = 13;
int ledSala = 12;
int ledComedor = 11;

//Led RGB
int ledR = 10;
int ledG = 9;
int ledB = 8;

//Servo Motor
Servo servo;
bool PuertaAbierta = false;
unsigned long TiempoPuerta = 0;
const unsigned long TiempoAbierta = 5000;

//Sensor Proximidad
const int trigger = 6;
const int echo = 5;
long distancia = 0;

//Motor Garage
bool GarageAbierto = false;
bool ObjetoPresente = false;
unsigned long TiempoGarage = 0;
const unsigned long TiempoGAbierto = 8000;
const int pinesMotor[] = {A2, A3, A4, A5};

const int pasos[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};

int velocidad = 2000;      // menor = más rápido
int pasosVuelta = 4076;   // 28BYJ-48 aprox. una vuelta

//Foto Resistor
  const int foto = A0;     
  int luz = 0;

  int ledExt = 4;

void setup() {

  //Modulo Bluetooth
  Serial.begin(9600);
  BT.begin(9600);

  Serial.println("Bluetooth listo.");
  Serial.println("Esperando datos...");

  //Leds Cuartos
  pinMode (ledCocina, OUTPUT);
  pinMode (ledComedor, OUTPUT);
  pinMode (ledSala, OUTPUT);

  //LedRGB
  pinMode (ledR, OUTPUT);
  pinMode (ledG, OUTPUT);
  pinMode (ledB, OUTPUT);

  //Servo Motor
  servo.attach (7);
  servo.write(0);

  //Sensor Proximidad
  pinMode (trigger, OUTPUT);
  pinMode (echo, INPUT);
  digitalWrite (trigger, LOW);

  //Motor Garage
  for (int i = 0; i < 4; i++) {
    pinMode(pinesMotor[i], OUTPUT);
  }
  
}

void loop() {

  Bluetooth();
  AbrirPuerta();
  LecturaSensor();
  ControlarGarage();
  Leerluz();
  ControlarLuzExterior();


  if (Serial.available()) {
    char c = Serial.read();
    BT.write(c);
  }

}

void Bluetooth() {

  if (BT.available()) {

    String dato = BT.readString(); 
    dato.trim(); 

    Serial.print("Recibido: ");
    Serial.println(dato);

    EjecutarDato(dato);

  }

}

void EjecutarDato(String dato) {

  if (dato == "enciende cocina") {
      digitalWrite(ledCocina, HIGH);
      BT.println("ledCocina ENCENDIDO");
    } else if (dato == "apaga cocina") {
      digitalWrite(ledCocina, LOW);
      BT.println("ledCocina APAGADO");
    } else if (dato == "enciende comedor") {
      digitalWrite(ledComedor, HIGH);
      BT.println("ledComedor PRENDIDO");
    } else if (dato == "apaga comedor") {
      digitalWrite(ledComedor, LOW);
      BT.println("ledComedor APAGADO");
    } else if (dato == "enciende sala") {
      digitalWrite(ledSala, HIGH);
      BT.println("ledSala PRENDIDO");
    } else if (dato == "apaga sala") {
      digitalWrite(ledSala, LOW);
      BT.println("ledSala APAGADO");
    } else if (dato == "enciende cuarto") {
      digitalWrite (ledR, HIGH);
      digitalWrite (ledG, HIGH);
      digitalWrite (ledB, HIGH);
    } else if (dato == "apaga cuarto") {
      digitalWrite (ledR, LOW);
      digitalWrite (ledG, LOW);
      digitalWrite (ledB, LOW);
    } else if (dato == "modo romance") {
      digitalWrite (ledR, HIGH);
      digitalWrite (ledG, LOW);
      digitalWrite (ledB, LOW);      
    } else if (dato == "abre puerta") {
      PuertaAbierta = true;
      TiempoPuerta = millis();
      servo.write (100);
    } else {
      BT.print("Recibi comando desconocido: ");
      BT.println(dato);
    }

}

void AbrirPuerta() {

  if (PuertaAbierta && millis() - TiempoPuerta >= TiempoAbierta) {
    servo.write (0);
    PuertaAbierta = false;
  }

}

void LecturaSensor() {

  unsigned long tiempoEco;

  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  tiempoEco = pulseIn(echo, HIGH, 30000);

  if (tiempoEco == 0) {
    // no hubo eco (fuera de rango) -> evita falsos positivos
    distancia = 999;
  } else {
    distancia = tiempoEco / 58;
  }

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

}

void ControlarGarage() {

  if (distancia <= 8) {
    ObjetoPresente = true;
  } else if (distancia > 10) {
    ObjetoPresente = false;
  }

  if (ObjetoPresente && !GarageAbierto) {
    Serial.println(">>> ABRIENDO GARAGE");
    girarMotor(1, pasosVuelta);
    TiempoGarage = millis();
    GarageAbierto = true;
  }

  // DEBUG: verifica si entra aquí
  if (GarageAbierto) {
    Serial.print("Esperando para cerrar... tiempo transcurrido: ");
    Serial.println(millis() - TiempoGarage);
  }

  if (GarageAbierto && millis() - TiempoGarage >= TiempoGAbierto) {
    Serial.println(">>> CERRANDO GARAGE");
    girarMotor(-1, pasosVuelta);
    GarageAbierto = false;
  }

}


void girarMotor(int direccion, int totalPasos) {

  static int pasoActual = 0;

  for (int i = 0; i < totalPasos; i++) {
    pasoActual += direccion;

    if (pasoActual > 7) pasoActual = 0;
    if (pasoActual < 0) pasoActual = 7;

    escribirPaso(pasoActual);
    delayMicroseconds(velocidad);
  }

  apagarMotor();

}

void escribirPaso(int paso) {

  for (int i = 0; i < 4; i++) {
    digitalWrite(pinesMotor[i], pasos[paso][i]);
  }

}

void apagarMotor() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(pinesMotor[i], LOW);
  }

}

void Leerluz() {

  luz = analogRead(foto);

  Serial.print("Luz: ");
  Serial.println(luz);

}

void ControlarLuzExterior() {

  if (luz < 93) {        
    digitalWrite(ledExt, HIGH);
  }
  else {
    digitalWrite(ledExt, LOW);
  }

}