#include <Adafruit_NeoPixel.h>

#define PIN1 4
#define PIN2 0
#define NUMPIXELS 40

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUMPIXELS, PIN1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS, PIN2, NEO_GRBW + NEO_KHZ800);

const uint8_t colors[][3] = {
  {255, 100, 0},   // Amarillo
  {255, 60, 5},    // Rojo-anaranjado
  {255, 40, 5},    // Rojo naranjo
  {255, 20, 5},
  {255, 15, 5},    // Rojo
  {255, 10, 5},    // Rojo
  {255, 5, 5},     // Rojo
  {255, 5, 10},    // Rojo
  {255, 5, 15},    // Rojo
  {255, 10, 15},
  {255, 15, 30},   // Rojo rosado
  {255, 15, 50},   // Rosa fuerte
  {255, 30, 100}   // Magenta
};
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);

void setup() {
  strip1.begin();
  strip1.show(); // Inicializa todos los píxeles a 'apagado'
  strip2.begin();
  strip2.show(); // Inicializa todos los píxeles a 'apagado'
}

void loop() {
  intenseBlinkPattern();
}

void intenseBlinkPattern() {
  // Recorre los colores en orden ascendente
  for (int i = 0; i < numColors; i++) {
    changeColor(colors[i]);
  }

  // Recorre los colores en orden descendente, excepto el primero y el último
  for (int i = numColors - 2; i > 0; i--) {
    changeColor(colors[i]);
  }
}

void changeColor(const uint8_t color[3]) {
  for (int j = 0; j < 5; j++) { // Cambia rápidamente entre brillos
    int brightness = random(10, 50);    // Genera un brillo aleatorio entre 10 y 50

    setAllPixelsColor(color, brightness);
    delay(100);  // Mantiene el color y el brillo durante 100 ms
    setAllPixelsColor(color, 0);  // Apaga los píxeles rápidamente
    delay(100);  // Pausa corta antes de cambiar al siguiente color
  }
}

void setAllPixelsColor(const uint8_t color[3], uint8_t brightness) {
  // Ajusta los colores RGB y deja el componente blanco en 0
  uint32_t rgbColor = strip1.Color(color[0] * brightness / 255, color[1] * brightness / 255, color[2] * brightness / 255);

  for (int i = 0; i < NUMPIXELS; i++) {
    strip1.setPixelColor(i, rgbColor);
    strip2.setPixelColor(i, rgbColor);
  }

  strip1.show();
  strip2.show();
}
