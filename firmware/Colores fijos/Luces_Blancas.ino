
#include <Adafruit_NeoPixel.h>

#define PIN1 4
#define PIN2 0
#define NUMPIXELS 40

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUMPIXELS, PIN1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS, PIN2, NEO_GRBW + NEO_KHZ800);

void setup() {
  strip1.begin();
  strip1.show(); // Inicializa todos los píxeles a 'apagado'
  strip2.begin();
  strip2.show(); // Inicializa todos los píxeles a 'apagado'
}

void loop() {
  fadeToWhite(13); // Velocidad del fade in/out
}

void fadeToWhite(uint8_t fadeSpeed) {
  for(int brightness = 5; brightness <= 100; brightness++) {
    setAllPixelsWhite(brightness);
    delay(fadeSpeed);
  }

  for(int brightness = 100; brightness >= 5; brightness--) {
    setAllPixelsWhite(brightness);
    delay(fadeSpeed);
  }
}

void setAllPixelsWhite(uint8_t brightness) {
  uint32_t color = strip1.Color(0, 0, 0, brightness); // Blanco ajustando el canal W

  for(int i = 0; i < NUMPIXELS; i++) {
    strip1.setPixelColor(i, color);
    strip2.setPixelColor(i, color);
  }

  strip1.show();
  strip2.show();
}
