#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
//#include <ESP8266WiFi.h>

typedef struct struct_message {  	
    int r;
    int g;
  	int b;
  	float a;
  	bool conectado;
	  int tipo;
} struct_message;

struct_message myData;

#define LED_PIN_BI 4
#define LED_PIN_BD 0


#define NUM_LEDS_BI 31
#define NUM_LEDS_BD 31
#define NUM_LEDS 31

int intensidad = 50;

Adafruit_NeoPixel strip_BI = Adafruit_NeoPixel(NUM_LEDS_BI, LED_PIN_BI, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel strip_BD = Adafruit_NeoPixel(NUM_LEDS_BD, LED_PIN_BD, NEO_RGBW + NEO_KHZ800);

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
	memcpy(&myData, incomingData, sizeof(myData));
  	Serial.print("Bytes received: ");
  	Serial.println(len);
  	Serial.print("Intensidad: ");
  	Serial.println(myData.a);
  	
  	Serial.print(" Red: ");
  	Serial.println(myData.r);
  	
  	Serial.print("Green: ");
  	Serial.println(myData.g);

    Serial.print(" Blue: ");
    Serial.println(myData.b);
    
  	Serial.print("conectado: ");
  	Serial.println(myData.conectado);
  	Serial.println();

  	intensidad = map(int(myData.a), 3, 100, 0, 255);
  	if (intensidad > 255) intensidad = 255;
  	if (intensidad < 0) intensidad = 0;
}

void setup()
{
  	// Initialize Serial Monitor
  	Serial.begin(115200);

  	// Set device as a Wi-Fi Station
  	WiFi.mode(WIFI_STA);

  	// Init ESP-NOW
  	if (esp_now_init() != ESP_OK)
	  {
    	Serial.println("Error initializing ESP-NOW");
    	return;
  	}

  	// Once ESPNow is successfully Init, we will register for recv CB to
  	// get recv packer info
  	//esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  	//esp_now_register_recv_cb(OnDataRecv);
  	// Once ESPNow is successfully Init, we will register for recv CB to
  	// get recv packer info
  	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  	strip_BI.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  	strip_BI.show();   // Turn OFF all pixels ASAP
  	strip_BI.setBrightness(intensidad);

  	strip_BD.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  	strip_BD.show();   // Turn OFF all pixels ASAP
  	strip_BD.setBrightness(intensidad);
 	
}



void loop() {

	if (myData.conectado == false)
	{
    	// blanco
    	fullColor(1, strip_BI.Color(0, 0, 0, 100));  // True white (not RGB white)
    	fullColor(2, strip_BD.Color(0, 0, 0, 100));  // True white (not RGB white)
    	strip_BI.setBrightness(intensidad);
    	strip_BD.setBrightness(intensidad);
 	}
  else
	{
      	if(myData.tipo == 1)
      	{ 
      	    colorWipe((myData.g * intensidad/255),(myData.r * intensidad/255), (myData.b * intensidad/255), 50);
            fullColor(1, strip_BI.Color(0,0,0,0);  // True white (not RGB white)
            fullColor(2, strip_BD.Color(0,0,0,0);  // True white (not RGB white)
             
      	}
        if(myData.tipo == 6)
        { 
            setAll(1, (myData.r * intensidad/255),(myData.g * intensidad/255), (myData.b * intensidad/255));
            setAll(2, (myData.r * intensidad/255),(myData.g * intensidad/255), (myData.b * intensidad/255)) ;      
        }
        
  }
  delay(100);
}

void colorWipe(byte red, byte green, byte blue, int SpeedDelay)
{
  for(uint16_t i=0; i<NUM_LEDS; i++) 
  {
      setPixel(1,i, red, green, blue);
      setPixel(2,i, red, green, blue);
      showStrip(1);
      showStrip(2);
      delay(SpeedDelay);
  }
}

void showStrip(int ch)
{
	if(ch == 1) strip_BI.show();
 	if(ch == 2) strip_BD.show();
}

void setPixel(int ch, int Pixel, byte red, byte green, byte blue)
{
	if(ch == 1) strip_BI.setPixelColor(Pixel, strip_BI.Color( green,red, blue));
	if(ch == 2) strip_BD.setPixelColor(Pixel, strip_BD.Color( green,red, blue));
	}

void setAll(int ch, byte red, byte green, byte blue)
{
	for(int i = 0; i < NUM_LEDS; i++ )
	{
    	setPixel(ch,i, red, green, blue);
  	}
  	showStrip(ch);
}

void colorWipe(uint32_t color, int wait)
{
  for (int i = 0; i < strip_BI.numPixels(); i++) {  // For each pixel in strip...
    strip_BI.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    strip_BI.show();                                //  Update strip to match
    delay(wait);                                    //  Pause for a moment
  }
}

void fullColor(uint8_t ch, uint32_t color) {
  if (ch == 1) {
    for (int i = 0; i < strip_BI.numPixels(); i++) { strip_BI.setPixelColor(i, color); }
    strip_BI.show();  //  Update strip to match
  }
  if (ch == 2) {
    for (int i = 0; i < strip_BD.numPixels(); i++) { strip_BD.setPixelColor(i, color); }
    strip_BD.show();  //  Update strip to match
  }
 
}
