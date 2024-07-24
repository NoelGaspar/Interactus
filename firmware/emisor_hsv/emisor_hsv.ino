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
#include <Coordinates.h>


#define TEMP_MAX  36
#define TEMP_MIN  26
#define BPM_MAX   100
#define BPM_MIN   60


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

//Conversor de coordenadas
Coordinates point = Coordinates();

float x = 0.0;
float y = 0.0;
float r = 0.0;
float theta = 0.0;
float z = 0.0;
float theta_aux = 0.0;


// PRINT
unsigned long timer = millis();

// ESPNOW
uint8_t broadcastAddress[] = { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }; // PRENDA{ 0x48, 0x27, 0xE2, 0x4D, 0xE6, 0xEC }  //LUCES CHIKAS { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }

// Structure example to send data
typedef struct struct_message {
  	long h;
  	int s;
  	int v;
  	float a;
  	bool conectado;
	  int tipo;
} struct_message;

struct_message myData;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  // callback when data is sent
  //Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    //Serial.println("Delivery success");
  } else {
    //Serial.println("Delivery fail");
  }
}

unsigned long delta_tiempo = 0;
long h = 0;
int s = 0;
int v = 0;
int tipo  = 0;
int zona = 0;

void conversor(float _temp, float _bpm);


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
    conversor(tempActual,beatAvg);
  	myData.h = h;
  	myData.s = s;
  	myData.v = v;
  	myData.a = accActual;
	  myData.tipo = z;
	  myData.conectado = pulseConnected;
}

void sendESPNow()
{
	esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
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
   	Serial.print(" Acc: ");
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
   	Serial.print(" Pulse: ");
    Serial.print(beatAvg);
}

void conversor(float _temp, float _bpm)
{
  float t_aux = _temp;
  float b_aux = _bpm;
  if(t_aux < TEMP_MIN){t_aux = TEMP_MIN;}
  if(t_aux > TEMP_MAX){t_aux = TEMP_MAX;}
  if(b_aux < BPM_MIN ){b_aux = BPM_MIN;}  
  if(b_aux > BPM_MAX ){b_aux = BPM_MAX;}
  
  x = (t_aux / 5.0) - (31/5.0);
  y = (b_aux/20.0)-4;
  x= -1*x;
  y = -1*y;
  
 // r = pow( (pow(x,2)+pow(y,2)),0.5);
 // theta = atan(y/x);
  point.fromCartesian(x,y);
  r = point.getR();
  r = r/sqrt(2);
  theta = point.getAngle();
  theta = theta *180.0/PI;

  theta_aux = theta + 45;
  if(theta_aux < 180){z = theta_aux;}
  else{ z = abs(360 - theta_aux); }
  
  s = int(r*255);
  h = long( (65535/360.0)*theta );
   
  Serial.print(" x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" r: ");
  Serial.print(r);
  Serial.print(" th: ");
  Serial.print(theta);
  Serial.print(" s: ");
  Serial.print(s);
  Serial.print(" h: ");  
  Serial.print(h);
  Serial.print(" z: ");  
  Serial.println(z);
  
    

}
