#include <BluetoothSerial.h>
#include <CD74HC4067.h>

#define RPWM_2 4
#define LPWM_2 25

#define RPWM_1 26
#define LPWM_1 27

#define pinBoton 13

CD74HC4067 my_mux(17, 5, 18, 19);

const int SIG = 36;

BluetoothSerial SerialBT;

const uint8_t SensorCount = 8;

uint16_t sensorValues[SensorCount];

int sensorMin[8];
int sensorMax[8];

float P;
float D;
float lastError = 0;

float Kp = 0.08;
float Kd = 0.60;

bool lastButton = HIGH;
int estado = 0;

void Bluetooth() {

  if (SerialBT.available()) {

    char dato = SerialBT.read();

    if (dato == 'A') Kp += 1;
    else if (dato == 'S') Kp += 0.1;

    else if (dato == 'D') Kp -= 1;
    else if (dato == 'F') Kp -= 0.1;

    else if (dato == 'G') Kd += 1;
    else if (dato == 'H') Kd += 0.1;

    else if (dato == 'J') Kd -= 1;
    else if (dato == 'K') Kd -= 0.1;

    SerialBT.print("Kp: ");
    SerialBT.print(Kp);
    SerialBT.print("  Kd: ");
    SerialBT.println(Kd);
  }
}

void leerSensores() {

  for (int i = 0; i < 8; i++) {

    my_mux.channel(i);

    delayMicroseconds(10);

    analogRead(SIG);

    delayMicroseconds(5);

    int suma = 0;

    for (int j = 0; j < 3; j++) {
      suma += analogRead(SIG);
    }

    sensorValues[i] = suma / 3;
  }
}

void normalizarSensores() {

  for (int i = 0; i < 8; i++) {
    if(SensorValues[i] > 800) SensorValues[i] = 1000;
 
  else{
    SensorValues[i] = 0;
 }

}

void mostrarSensores() {
  for (int i = 0; i < 8; i++) {

    Serial.print(sensorValues[i]);
    Serial.print('\t');
  }

  Serial.println();
}

void manejarBoton() {

  bool actual = digitalRead(pinBoton);

  if (lastButton == HIGH && actual == LOW) {

    delay(50);

    if (estado == 0) {

      calibrarSensores();

      estado = 1;

      Serial.println("Presiona nuevamente para iniciar");
    }

    else if (estado == 1) {

      estado = 2;

      Serial.println("Robot iniciado");
    }
  }

  lastButton = actual;
}

void ADELANTE(int velIzq, int velDer) {

  ledcWrite(RPWM_2, velIzq);
  ledcWrite(LPWM_2, 0);

  ledcWrite(RPWM_1, velDer);
  ledcWrite(LPWM_1, 0);
}


int pesos[8] = {
    -3500,
    -2500,
    -1500,
    -500,
     500,
     1500,
     2500,
     3500
  };

void PID_control() {

  leerSensores();

  normalizarSensores();

  long suma = 0;
  long total = 0;

  

  for (int i = 0; i < 8; i++) {

    suma += (long)sensorValues[i] * pesos[i];
    total += sensorValues[i];
  }

  if (total == 0) return;

  int position = suma / total;

  int error = position;

  P = error;
  D = error - lastError;

  lastError = error;

  int correction = (P * Kp) + (D * Kd);

  int baseSpeed = 150;

  int leftMotorSpeed = baseSpeed + correction;
  int rightMotorSpeed = baseSpeed - correction;

  leftMotorSpeed = constrain(leftMotorSpeed, 0, 220);
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 220);

  ADELANTE(leftMotorSpeed, rightMotorSpeed);
}

void setup() {

  Serial.begin(115200);

  SerialBT.begin("ESP32_Seguidor");

  pinMode(pinBoton, INPUT_PULLUP);

  pinMode(SIG, INPUT);

  ledcAttach(RPWM_2, 5000, 8);
  ledcAttach(LPWM_2, 5000, 8);

  ledcAttach(RPWM_1, 5000, 8);
  ledcAttach(LPWM_1, 5000, 8);

  Serial.println("Listo para calibrar");
}

void loop() {
  Bluetooth();
  manejarBoton();

  if (estado == 2) {

    PID_control();
    mostrarSensores();
  }
}