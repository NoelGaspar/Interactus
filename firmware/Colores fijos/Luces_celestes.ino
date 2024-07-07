#include <Adafruit_NeoPixel.h>

#define PIN1 4
#define PIN2 0
#define NUMPIXELS 40

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUMPIXELS, PIN1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS, PIN2, NEO_GRBW + NEO_KHZ800);

const uint8_t colors[][3] = {
  {171, 220, 48},    // Amarillo-verde
  {132, 202, 49},    // Verde
  {88, 184, 52},     // Verde oscuro
  {23, 161, 86},     // Verde azulado
  {25, 170, 121},    // Verde azul claro
  {30, 187, 182},    // Azul verdoso
  {28, 174, 250},    // Azul claro
  {23, 137, 252},    // Azul
  {15, 79, 251}      // Azul oscuro
};
const uint8_t numColors = sizeof(colors) / sizeof(colors[0]);
const uint8_t fadeSteps = 100; // Pasos de transición (más pasos = transición más suave)
const uint8_t fadeDelay = 35;  // Retraso entre pasos de transición (ajustar para velocidad de transición)
const uint8_t maxBrightness = 200; // Máximo brillo para simular la inhalación
const uint8_t minBrightness = 5;   // Brillo mínimo para los fades

void setup() {
  strip1.begin();
  strip1.show(); // Inicializa todos los píxeles a 'apagado'
  strip2.begin();
  strip2.show(); // Inicializa todos los píxeles a 'apagado'
}

void loop() {
  breatheEffect();
}

void breatheEffect() {
  for (int i = 0; i < numColors; i++) {
    fadeToColor(colors[i], maxBrightness);
    delay(fadeDelay);
    fadeOutColor(colors[i], maxBrightness); // Disminuye hasta 50 para simular la exhalación
    delay(fadeDelay);
  }
}

void fadeToColor(const uint8_t color[3], uint8_t maxBrightness) {
  // Calcula la transición de color gradual con efecto de respiración
  for (int step = 0; step <= fadeSteps; step++) {
    uint8_t brightness = map(step, 0, fadeSteps, minBrightness, maxBrightness);
    setAllPixelsColor(color, brightness);
    delay(fadeDelay);
  }
}

void fadeOutColor(const uint8_t color[3], uint8_t maxBrightness) {
  // Fade out gradual al disminuir la intensidad de color hasta minBrightness
  for (int step = fadeSteps; step >= 0; step--) {
    uint8_t brightness = map(step, 0, fadeSteps, minBrightness, maxBrightness);
    setAllPixelsColor(color, brightness);
    delay(fadeDelay);
  }
}

void setAllPixelsColor(const uint8_t color[3], uint8_t brightness) {
  // Establece el color en todas las tiras de LEDs
  uint32_t rgbColor = strip1.Color(color[0] * brightness / 255, color[1] * brightness / 255, color[2] * brightness / 255, 0);

  for (int i = 0; i < NUMPIXELS; i++) {
    strip1.setPixelColor(i, rgbColor);
    strip2.setPixelColor(i, rgbColor);
  }

  strip1.show();
  strip2.show();
}
