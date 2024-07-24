#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
//#include <ESP8266WiFi.h>

typedef struct struct_message {  	
    long h;
    int s;
  	int v;
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
int color_r = 0;
int color_b = 0;
int color_g = 0;

long color_h = 0;
int color_s = 0;
int color_v = 0;

long diff = 0;

Adafruit_NeoPixel strip_BI = Adafruit_NeoPixel(NUM_LEDS_BI, LED_PIN_BI, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel strip_BD = Adafruit_NeoPixel(NUM_LEDS_BD, LED_PIN_BD, NEO_RGBW + NEO_KHZ800);

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
	memcpy(&myData, incomingData, sizeof(myData));
  	Serial.print("Bytes received: ");
  	Serial.println(len);
  	
  	Serial.print(" Hue: ");
  	Serial.println(myData.h);
  	
  	Serial.print(" Saturation: ");
  	Serial.println(myData.s);

    Serial.print(" Value: ");
    Serial.println(myData.v);
    
  	Serial.print("conectado: ");
  	Serial.println(myData.conectado);
  	
    Serial.print("zona:");
    Serial.println(myData.tipo);
    

  	intensidad = map(int(myData.a)*10, 0, 30, 60, 255);
  	if (intensidad > 255) intensidad = 255;
  	if (intensidad < 0) intensidad = 0;
    
    Serial.print("Intensidad: ");
    Serial.println(intensidad);    
    Serial.println();

    //color_r = myData.r * intensidad/255;
    //color_g = myData.g * intensidad/255;
    //color_b = myData.b * intensidad/255;
    color_h = myData.h;
    color_s = myData.s;
    color_v = intensidad;
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
      RunningLights(0xff,0xff,0xff, 60);  // white
 	}
  else
	{
      	
      	    if(myData.tipo >90)
            {
              setAllHSV(1, color_h,color_s, color_v);
              setAllHSV(2, color_h,color_s, color_v);
      	      delay((180-myData.tipo)*3);
              setAll(1, 0,0,0);
              setAll(2, 0,0,0);
              delay((180-myData.tipo));
            }
            else
            {
              setAllHSV(1, color_h,color_s, color_v);
              setAllHSV(2, color_h,color_s, color_v);
             
            }
        
        
  }
  delay(100);
}





void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
 
  for(int j=0; j<NUM_LEDS/2; j++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(1,i,((sin(i+Position) * 127 + 128)/255)*red,
                     ((sin(i+Position) * 127 + 128)/255)*green,
                     ((sin(i+Position) * 127 + 128)/255)*blue);
        
        setPixel(2,i,((sin(i+Position) * 127 + 128)/255)*red,
                     ((sin(i+Position) * 127 + 128)/255)*green,
                     ((sin(i+Position) * 127 + 128)/255)*blue);
      }
     
      showStrip(1);
      showStrip(2);
      delay(WaveDelay);
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

void setPixelHSV(int ch, int Pixel, long h, int s, int v)
{
  if(ch == 1) strip_BI.setPixelColor(Pixel, strip_BI.gamma32(strip_BI.ColorHSV( h, s, v)));
  if(ch == 2) strip_BD.setPixelColor(Pixel, strip_BD.gamma32(strip_BD.ColorHSV( h, s, v)));
}

void setAll(int ch, byte red, byte green, byte blue)
{
	for(int i = 0; i < NUM_LEDS; i++ )
	{
    	setPixel(ch,i, red, green, blue);
  	}
  	showStrip(ch);
}

void setAllHSV(int ch, long h ,int s, int v)
{
  for(int i = 0; i < NUM_LEDS; i++ )
  {
      setPixelHSV(ch,i, h, s, v);
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
