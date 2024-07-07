/* TODO:

AC: calibración, es necesario cada vez.

*/
#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_BMP280.h>  // Temperatura, presion humedad
#include "MAX30105.h"         // Pulso bpm
#include <MPU6050_light.h>    //acelerometro
#include "heartRate.h"        // ritmo cardiaco
#include <ESP8266WiFi.h>
#include <espnow.h>

// Pulso
MAX30105 pulseSensor;
const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred

long pulseRaw;
bool pulseConnected = false;
float beatsPerMinute;
int beatAvg;

// TEMPERATURA
Adafruit_BMP280 tempSensor;
bool tempStatus;
float tempActual = 0.0;

// ACELEROMETRO
MPU6050 acelSensor(Wire);
byte accStatus;
float accX = 0.0;
float accY = 0.0;
float accZ = 0.0;
float accActual = 0.0;

// PRINT
unsigned long timer = millis();

// ESPNOW
uint8_t broadcastAddress[] = { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }; // PRENDA{ 0x48, 0x27, 0xE2, 0x4D, 0xE6, 0xEC }  //LUCES CHIKAS { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }

// Structure example to send data
typedef struct struct_message {
  	int r;
  	int g;
  	int b;
  	float a;
  	bool conectado;
	int tipo;
} struct_message;

struct_message myData;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  // callback when data is sent
  //Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    //Serial.println("Delivery success");
  } else {
    //Serial.println("Delivery fail");
  }
}

unsigned long delta_tiempo = 0;
int r = 0;
int g = 0;
int b = 0;
int tipo  = 0;
void cuadricula(float _temp, float _bpm);

void setup()
{
	Serial.begin(115200);
  	Wire.begin();

	// ACELEROMETRO
  	accStatus = acelSensor.begin();

	Serial.print(F("MPU6050 status: "));
	Serial.println(accStatus);

	while (accStatus != 0) {};  // stop everything if could not connect to MPU6050

	Serial.println(F("Calculating offsets, do not move MPU6050"));
	delay(1000);
	acelSensor.calcOffsets(true, true);  // gyro and accelero
	Serial.println("Done!\n");

	while (!Serial) delay(100);
	// TEMPERATURA
	tempStatus = tempSensor.begin();
	if (!tempStatus)
	{
    	Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    	Serial.print("SensorID was: 0x");
    	Serial.println(tempSensor.sensorID(), 16);
    	Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    	Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    	Serial.print("        ID of 0x60 represents a BME 280.\n");
    	Serial.print("        ID of 0x61 represents a BME 680.\n");
    	while (1) delay(10);
	}

	/* Default settings from datasheet. */
  	tempSensor.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                         Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                         Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                         Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                         Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

 	// PULSO
	if (!pulseSensor.begin(Wire, I2C_SPEED_FAST))
	{
    	Serial.println("MAX30105 was not found. Please check wiring/power. ");
    	while (1)
      	;
  	}

	Serial.println("Place your index finger on the sensor with steady pressure.");
 	pulseSensor.setup();                     //Configure sensor with default settings
 	pulseSensor.setPulseAmplitudeRed(0x0A);  //Turn Red LED to low to indicate sensor is running
 	pulseSensor.setPulseAmplitudeGreen(0);   //Turn off Green LED

	//WiFi
	WiFi.mode(WIFI_STA);
	// Init ESP-NOW
  	if (esp_now_init() != 0)
	{
    	Serial.println("Error initializing ESP-NOW");
    	return;
  	}
  	//attachInterrupt(digitalPinToInterrupt(D5), detectsMovement, FALLING);

  	// Once ESPNow is successfully Init, we will register for Send CB to
  	// get the status of Trasnmitted packet
  	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  	esp_now_register_send_cb(OnDataSent);

  	// Register peer
  	esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop()
{
	readTemp();
 	readAcel();
 	readPulse();
 	updateMsg();
 	sendESPNow();
 	//imprimir();

}

void updateMsg()
{
 	cuadricula(tempActual, beatAvg);
  	myData.r = r;
  	myData.g = g;
  	myData.b = b;
  	myData.a = accActual;
	myData.tipo = tipo;
  	myData.conectado = pulseConnected;
}

void sendESPNow()
{
	esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}

void imprimir()
{
  	if (millis() - timer > 1000UL)
	{  // print data every second

    	int r = myData.r;
    	Serial.print(r);
  	}
  	timer = millis();
}

void readTemp()
{
  	tempActual = tempSensor.readTemperature();
  	Serial.print("\t Temp: ");
  	Serial.print(tempActual);
}

void readAcel()
{
  	acelSensor.update();
  	accX = acelSensor.getAccX();
  	accY = acelSensor.getAccY();
  	accZ = acelSensor.getAccZ();
  	accActual = sqrt(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2));
   	Serial.print("Acc: ");
  	Serial.print(accActual);
}

void readPulse()
{
	long pulseRaw = pulseSensor.getIR();

  	if (pulseRaw < 50000)
	{
    	Serial.println("No se detecta dedo");
    	pulseConnected = false;
  	}
	else
	{
    	pulseConnected = true;
  	}
	if (checkForBeat(pulseRaw) == true)
	{

    	long delta = millis() - lastBeat;
    	lastBeat = millis();

    	beatsPerMinute = 60 / (delta / 1000.0);

    	if (beatsPerMinute < 255 && beatsPerMinute > 20)
		{
      		rates[rateSpot++] = (byte)beatsPerMinute;  //Store this reading in the array
      		rateSpot %= RATE_SIZE;                     //Wrap variable

      		//Take average of readings
      		beatAvg = 0;
      		for (byte x = 0; x < RATE_SIZE; x++)
        		beatAvg += rates[x];
      		beatAvg /= RATE_SIZE;
    	}
  	}
   	Serial.print("\t Pulse: ");
    Serial.println(beatAvg);
}

void cuadricula(float _temp, float _bpm) {
  //1er cuadrante
  	if (_bpm > 140) {
    	if ((_temp > 22) and (_temp <= 25))
		{
      		Serial.println("cuadrante 1");
      		r = 35;
      		g = 205;
      		b = 52;
			tipo  = 1;
    	}
		else if ((_temp > 25) and (_temp <= 28))
		{
      		Serial.println("cuadrante 2");
      		r = 23;
      		g = 137;
      		b = 252;
			tipo  = 1;
    	}
		else if ((_temp > 28) and (_temp <= 31))
		{
      		Serial.println("cuadrante 3");
      		r = 15;
      		g = 79;
      		b = 252;
			tipo  = 2;
    	} else if ((_temp > 31) and (_temp <= 34)) {
      	Serial.println("cuadrante 4");
      	r = 13;
      	g = 51;
      	b = 251;
		tipo = 2;
    	} else if (_temp > 34) {
      	Serial.println("cuadrante 5");
      	r = 43;
      	g = 36;
      	b = 251;
		tipo =2;
    	}
  } else if ((_bpm > 120) and (_bpm <= 140)) {
    if (_temp < 22) {
      Serial.println("cuadrante 6");
      r = 27;
      g = 179;
      b = 154;
		tipo = 1;
    } else if ((_temp > 22) and (_temp <= 25)) {
      Serial.println("cuadrante 7");
      r = 33;
      g = 197;
      b = 220;
tipo = 1;
    } else if ((_temp > 25) and (_temp <= 28)) {
      Serial.println("cuadrante 8");
      r = 28;
      g = 174;
      b = 250;
		tipo = 1;
    } else if ((_temp > 28) and (_temp <= 31)) {
      Serial.println("cuadrante 9");
      r = 15;
      g = 79;
      b = 251;
		tipo = 2;
    } else if ((_temp > 31) and (_temp <= 34)) {
      Serial.println("cuadrante 10");
      r = 43;
      g = 36;
      b = 251;
		tipo = 2;
    } else if ((_temp > 34) and (_temp <= 37)) {
      Serial.println("cuadrante 11");
      r = 84;
      g = 36;
      b = 248;
		tipo = 3;
    } else if ((_temp > 37)) {
      Serial.println("cuadrante 12");
      r = 123;
      g = 37;
      b = 243;
		tipo = 3;
    }
  } else if ((_bpm > 110) and (_bpm <= 120)) {
    if (_temp < 22) {
      Serial.println("cuadrante 13");
      r = 23;
      g = 161;
      b = 86;
		tipo = 1;
    } else if ((_temp > 22) and (_temp <= 25)) {
      Serial.println("cuadrante 14");
      r = 25;
      g = 170;
      b = 121;
		tipo = 1;
    } else if ((_temp > 25) and (_temp <= 28)) {
      Serial.println("cuadrante 15");
      r = 30;
      g = 187;
      b = 182;
		tipo = 1;
    } else if ((_temp > 28) and (_temp <= 31)) {
      Serial.println("cuadrante 16");
      r = 13;
      g = 35;
      b = 248;
		tipo = 2;
    } else if ((_temp > 31) and (_temp <= 34)) {
      Serial.println("cuadrante 17");
      r = 255;//166;
      g = 20;//38;
      b = 170;//250;
		tipo = 3;
    } else if ((_temp > 34) and (_temp <= 37)) {
      Serial.println("cuadrante 18");
      r = 211;//211;
      g = 10;//38;
      b = 150;//247;
		tipo = 3;
    } else if ((_temp > 37)) {
      Serial.println("cuadrante 19");
      r = 255;//253;
      g = 0;//40;
      b = 130;//248;
		tipo = 3;
    }
  } else if ((_bpm > 100) and (_bpm <= 110)) {
    if (_temp < 22) {
      Serial.println("cuadrante 20");
      r = 48;
      g = 169;
      b = 52;
		tipo = 4;
    } else if ((_temp > 22) and (_temp <= 25)) {
      Serial.println("cuadrante 21");
      r = 120;//88;
      g = 255;//184;
      b = 25;//52;
		tipo = 4;
    } else if ((_temp > 25) and (_temp <= 28)) {
      Serial.println("cuadrante 22");
      r = 230;//171;
      g = 100;//214;
      b = 5;//48;
		tipo = 4;
    }
	else if ((_temp > 28) and (_temp <= 31)) {
      Serial.println("cuadrante 23");
      r = 255;//246;
      g = 0;//15;
      b = 0;//52;
		tipo = 6;
    } else if ((_temp > 31) and (_temp <= 34)) {
      Serial.println("cuadrante 24");
      r = 255;//251;
      g = 0;//23;
      b = 30;//131;
		tipo = 6;
    }
	else if ((_temp > 34) and (_temp <= 37))
	{
    	Serial.println("cuadrante 25");
      	r = 255;//253;
      	g = 0;//28;
      	b = 50;//170;
		tipo = 6;
    }
		else if ((_temp > 37))
		{
			Serial.println("cuadrante 26");
			r = 255;//253;
			g = 0;//34;
			b = 86;//212;
			tipo = 6;
    	}
  	}
	else if ((_bpm > 80) and (_bpm <= 110))
	{
    	if (_temp < 22)
		{
      		Serial.println("cuadrante 27");
      		r = 170;//132;
      		g = 170;//202;
      		b = 20;//49;
			tipo = 4;
    	}
		else if ((_temp > 22) and (_temp <= 25))
		{
      		Serial.println("cuadrante 28");
      		r = 230;//213;
      		g = 100;//236;
      		b = 5;//51;
			tipo = 4;
    	}
		else if ((_temp > 25) and (_temp <= 28))
		{
      		Serial.println("cuadrante 29");
      		r = 255;
      		g = 80;
      		b = 0;
			tipo = 5;
    	}
		else if ((_temp > 28) and (_temp <= 31))
		{
      		Serial.println("cuadrante 30");
      		r = 255;//253;
      		g = 10;//87;
      		b = 10;//32;
			tipo = 6;
    	}
		else if ((_temp > 31) and (_temp <= 34))
		{
      		Serial.println("cuadrante 31");
      		r = 255;//246;
      		g = 0;//15;
      		b = 0;//52;
			tipo = 6;
    	}
		else if ((_temp > 34) and (_temp <= 37))
		{
      		Serial.println("cuadrante 32");
      		r = 255;//248;
      		g = 0;//29;
      		b = 10;//88;
			tipo = 6;
    	}
 		else if ((_temp > 37)) {
      		Serial.println("cuadrante 33");
      r = 251;
      g = 23;
      b = 131;
		tipo = 6;
    	}
  	}
	else if ((_bpm < 80))
	{
    	if (_temp < 25) {
      	Serial.println("cuadrante 34");
      	r = 255//255;
      	g = 80//253;
      	b = 0//56;
		tipo = 5;
    }
	else if ((_temp > 25) and (_temp <= 28)) {
      	Serial.println("cuadrante 35");
      	r = 255;//253;
      	g = 50;//170;
      	b = 0;//43;
		tipo = 5;
    } else if ((_temp > 28) and (_temp <= 31)) {
      	Serial.println("cuadrante 36");
      	r = 255;//253;
      	g = 5;//87;
      	b = 5;//32;
		tipo = 6;
    } else if ((_temp > 31) and (_temp <= 34)) {
      Serial.println("cuadrante 37");
      r = 255;//250;
      g = 10;//45;
      b = 10;//27;
		tipo = 6;
    } else if ((_temp > 34)) {
      Serial.println("cuadrante 38");
      r = 255;//246;
      g = 0;//25;
      b = 0;//52;
		tipo = 6;
    }
  }
}
