/* TODO:

AC: calibración, es necesario cada vez.

*/
#include <Wire.h>
//#include <SPI.h>
//#include <Adafruit_BMP280.h>  // Temperatura, presion humedad
//#include "MAX30105.h"         // Pulso bpm
//#include <MPU6050_light.h>    //acelerometro
//#include "heartRate.h"        // ritmo cardiaco
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Coordinates.h>

#define TEMP_MAX 36
#define TEMP_MIN 26
#define BPM_MAX 120
#define BPM_MIN 60

//Conversor de coordenadas
Coordinates point = Coordinates();


float x = 0.0;
float y = 0.0;
float r = 0.0;
float theta = 0.0;

const byte RATE_SIZE = 4;  //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];     //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0;  //Time at which the last beat occurred

long pulseRaw;
bool pulseConnected = false;
float beatsPerMinute;
int beatAvg;

// TEMPERATURA
bool tempStatus;
float tempActual = 0.0;
float temp = 0.0;
float bmp = 0.0;
// ACELEROMETRO
byte accStatus;
float accX = 0.0;
float accY = 0.0;
float accZ = 0.0;
float accActual = 0.0;

int cuadrante = 0;
int intensidad = 0;
int tipo = 0;
int zona = 0;

// PRINT
unsigned long timer = millis();

// ESPNOW
uint8_t broadcastAddress[] = { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }; // PRENDA{ 0x48, 0x27, 0xE2, 0x4D, 0xE6, 0xEC }  //LUCES CHIKAS { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }
//uint8_t broadcastAddress[] = { 0x48, 0x27, 0xE2, 0x4D, 0xE6, 0xEC };  //LUCES CHIKAS { 0xA0, 0x76, 0x4E, 0x1C, 0x06, 0x70 }

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
long  h = 0;
int s = 0;
int v = 0;
int b = 0;
int t = 0;
void cuadricula(float _temp, float _bpm);

void setup()
{
	Serial.begin(115200);
  	Wire.begin();

	// ACELEROMETRO
  	//accStatus = acelSensor.begin();

	//Serial.print(F("MPU6050 status: "));
	//Serial.println(accStatus);

	while (!Serial) delay(100);
	// TEMPERATURA
  myData.conectado = true;
	//WiFi
	WiFi.mode(WIFI_STA);
	// Init ESP-NOW
  	if (esp_now_init() != 0)
	{
    	Serial.println("Error initializing ESP-NOW");
    	return;
  	}
  	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  	esp_now_register_send_cb(OnDataSent);

  	// Register peer
  	esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop()
{
	while(Serial.available()>0)
	{
		char tipo = Serial.read();
	  /*	if( tipo == 'c')
		{
			cuadrante = Serial.parseInt();
			Serial.print("configurando cuadrante. Nuevo cuadrante: ");
			Serial.println(cuadrante);
      		mapa(cuadrante);
      		tempActual  = temp ;
      		beatAvg = bmp;
      		updateMsg();
      		//sendESPNow();
		}*/
		// else
     
		if(tipo == 'h')
		{
			h = Serial.parseInt();
			Serial.print("Nuevo valor hue: ");
			Serial.println(h);
      		myData.h = h;
      		//updateMsg();
      		//sendESPNow();
		}
		else if(tipo == 's')
		{
			s = Serial.parseInt();
			Serial.print("Nuevo valor de saturación: ");
			Serial.println(s);
      		myData.s = s;
			//updateMsg();
      		//sendESPNow();
		}
		else if(tipo == 'v')
		{
			v = Serial.parseInt();
			Serial.print("Nuevo color azul: ");
			Serial.println(v);
      		myData.v = v;
      		//updateMsg();
      		//sendESPNow();
		}
		else if(tipo == 'i')
		{
			intensidad = Serial.parseInt();
			Serial.print("configurando intensisdad. Nueva intensidad: ");
			Serial.println(intensidad);
	      	accActual = intensidad;
			myData.a = accActual;
			//sendESPNow();
		}
		else if(tipo == 'z')
		{
			zona = Serial.parseInt();
			Serial.print("configurando zona. Nueva zona: ");
			Serial.println(zona);
			myData.tipo = zona;
			//sendESPNow();
		}
    else if(tipo == 't')
    {
      t = Serial.parseInt();
      Serial.print("configurando temperatura : ");
      Serial.println(t);
     
      //sendESPNow();
    }
    else if(tipo == 'b')
    {
      b = Serial.parseInt();
      Serial.print("configurando bpm: ");
      Serial.println(b);
      conversor(t,b);
      myData.h = h;
      myData.s = s;
      myData.v = v;    
      //sendESPNow();
    }
    else if(tipo == '\n')
    {
		  sendESPNow();
    }
	}


 	//imprimir();
  delay(100);

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
  y = (b_aux/30.0)-3;
  x= -1*x;
  y = -1*y;
 // r = pow( (pow(x,2)+pow(y,2)),0.5);
 // theta = atan(y/x);
  point.fromCartesian(x,y);
  r = point.getR();
  r = r/sqrt(2);
  theta = point.getAngle();
  theta = theta *180.0/PI;
  
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
  Serial.println(h);
    

}


/*
void mapa(int c)
{
  if (c == 1)
  {
      temp = 23;
      bmp = 150;
  }
  if (c == 2 )
  {
      temp = 27;
      bmp  = 150;
  }
  if (c == 3)
  {
      temp = 29;
      bmp = 150;
  }
  if (c == 4)
  {
      temp = 32;
      bmp = 150;
  }
  if (c == 5)
  {
      temp = 35;
      bmp = 150;
  }
  if (c == 6)
  {
      temp = 20;
      bmp = 130;
  }
  if (c == 7)
  {
      temp = 23;
      bmp  = 130;
  }
  if (c == 8)
  {
      temp = 27;
      bmp = 130;
  }
  if (c == 9)
  {
      temp = 29;
      bmp = 130;
  }
  if (c == 10)
  {
      temp = 33;
      bmp = 130;
  }
  if (c == 11)
  {
      temp = 36;
      bmp = 130;
  }
  if (c == 12)
  {
      temp = 39;
      bmp = 130;
  }
  if (c == 13)
  {
      temp = 6;
      bmp = 115;
  }
  if (c == 14)
  {
      temp = 23;
      bmp = 115;
  }
  if (c == 15)
  {
      temp = 26;
      bmp = 115;
  }
  if (c == 16)
  {
      temp = 29;
      bmp = 115;
  }
  if (c == 17)
  {
      temp = 32;
      bmp = 115;
  }
  if (c == 18)
  {
      temp = 36;
      bmp = 115;
  }
  if (c == 19)
  {
      temp = 38;
      bmp = 115;
  }
  if (c == 20)
  {
      temp = 20;
      bmp = 105;
  }
  if (c == 21)
  {
      temp = 23;
      bmp = 105;
  }
  if (c == 22)
  {
      temp = 26;
      bmp = 105;
  }
  if (c == 23)
  {
      temp = 29;
      bmp = 105;
  }
  if (c == 24)
  {
      temp = 32;
      bmp = 105;
  }
  if (c == 25)
  {
      temp = 36;
      bmp = 105;
  }
  if (c == 26)
  {
      temp = 38;
      bmp = 105;
  }
  if (c == 27)
  {
      temp = 18;
      bmp = 90;
  }
  if (c == 28)
  {
      temp = 23;
      bmp = 90;
  }
  if (c == 29)
  {
      temp = 26;
      bmp = 90;
  }
  if (c == 30)
  {
      temp = 29;
      bmp = 90;
  }
  if (c == 31)
  {
      temp = 32;
      bmp = 90;
  }
  if (c == 32)
  {
      temp = 36;
      bmp = 90;
  }
  if (c == 33)
  {
      temp = 38;
      bmp = 90;
  }
  if (c == 34)
  {
      temp = 22;
      bmp = 70;
  }
  if (c == 35)
  {
      temp = 26;
      bmp = 70;
  }
  if (c == 36)
  {
      temp = 30;
      bmp = 70;
  }
  if (c == 37)
  {
      temp = 32;
      bmp = 70;
  }
  if (c == 38)
  {
      temp = 36;
      bmp = 70;
  }
}




void updateMsg()
{
 	cuadricula(tempActual, beatAvg);
  	myData.r = r;
  	myData.g = g;
  	myData.b = b;
  	myData.a = accActual;
	myData.tipo = tipo;
  	myData.conectado = true;
}
*/

void sendESPNow()
{
  Serial.println("Enviando mensaje");
	esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}

void imprimir()
{
  	if (millis() - timer > 1000UL)
	{  // print data every second

    	//int r = myData.r;
    	//Serial.print(r);
  	}
  	timer = millis();
}

/*

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
      	r = 255;//255;
      	g = 80;//253;
      	b = 0;//56;
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

*/
