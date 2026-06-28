#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO_PANTALLA 128
#define ALTO_PANTALLA 32

#define BTN_NEXT 13

Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);

enum Pantallas {
  VELOCISTA,
  SUMO,
  DESPEJA_AREA
};

Pantallas pantallaActual = VELOCISTA;

bool modoSeleccionado = false;

unsigned long tiempoPresionado = 0;
bool botonEstabaPresionado = false;

void setup() {

  Serial.begin(115200);

  Wire.begin();

  pinMode(BTN_NEXT, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro la OLED");
    while (true);
  }

  Serial.println("OLED encontrada");

  display.clearDisplay();
  display.display();
}

void loop() {

  bool boton = digitalRead(BTN_NEXT) == LOW;

  if (boton && !botonEstabaPresionado) {
    tiempoPresionado = millis();
    botonEstabaPresionado = true;
  }

  if (!boton && botonEstabaPresionado) {

    unsigned long duracion = millis() - tiempoPresionado;

    if (!modoSeleccionado) {

      if (duracion < 2000) {
        pantallaActual = (Pantallas)((pantallaActual + 1) % 3);
      }
      else {
        modoSeleccionado = true;
      }

    }
    else {

      if (duracion >= 2000) {
        modoSeleccionado = false;
      }

    }

    botonEstabaPresionado = false;
  }

  if (!modoSeleccionado) {
    mostrarMenu();
  }
  else {
    mostrarModoSeleccionado();
  }
}

void mostrarMenu() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print(pantallaActual == VELOCISTA ? "> " : "  ");
  display.println("Velocista");

  display.print(pantallaActual == SUMO ? "> " : "  ");
  display.println("Sumo");

  display.print(pantallaActual == DESPEJA_AREA ? "> " : "  ");
  display.println("Despeja Area");

  display.display();
}

void mostrarModoSeleccionado() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 8);

  switch (pantallaActual) {

    case VELOCISTA:
      display.println("VELOCISTA");
      break;

    case SUMO:
      display.println("SUMO");
      break;

    case DESPEJA_AREA:
      display.setTextSize(1);
      display.println("DESPEJA");
      display.println("AREA");
      break;
  }

  display.display();
}