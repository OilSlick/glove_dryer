
#include "TH02_dev.h"
#include "Arduino.h"
#include "Wire.h"
#include "DHT.h" 

const int TIP120pin = 5;    //base pin of TIP120 transistor
const int ButtonPWR = 6;    //provide 5v to button circuit
const int inPin = 7;        //read button circuit status
int BUTTONVAL = 0;          //store button circuit status as value
int DRYMODE=0;              //determine monitor or dry mode (fan on or off)
int ONCE=0;                 //ensure FANCONTINUOUS() only runs once per drying cycle (per high humidity)
float LOWREADING=200;       //Track historical low RH
float HIGHREADING=0;        //Track historical high RH
long startTime ;            //Track fan runtime
long elapsedTime ;          //Track fan runtime
int FANPREV = 0;            //Store if fan has ever run

DHT dht;
  
//LCD requirements:

// You can use any (4 or) 5 pins 
#define sclk 13
#define mosi 11
#define cs   10
#define rst  9
#define dc   8

// Color definitions for LCD display
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);  

void setup()
{  
  //DHT sensor setup
  dht.setup(A0);
  
  //Button setup
  pinMode(inPin, INPUT);          // declare pushbutton as input
  pinMode(ButtonPWR,OUTPUT);      //provide power to button
  
  Serial.begin(9600);             // start serial for output

  //Temp+humid requirements
  Serial.println("****THO2 power up delay, letting voltage settle****\n");
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  Serial.println("Ready"); 

  //TIP120 Transistor base pin as OUTPUT
  pinMode(TIP120pin, OUTPUT);

  //LCD Display requirements
  display.begin();
  display.fillScreen(BLACK);
  display.setCursor(0,0);
  display.print("Glove Dryer V.1");
  delay(1000);
  display.fillScreen(BLACK);
}
 
void loop()
{ 
  //Button to activate LCD display
  digitalWrite(ButtonPWR, HIGH);
  BUTTONVAL = digitalRead(inPin);       // read button input value
  if (BUTTONVAL == LOW)                 // if button has been pushed
  {
  LCD_DISPLAY();
  }
  
  delay(dht.getMinimumSamplingPeriod()); //play nice with DHT sensor
  
  float humidityGLOVE = TH02.ReadHumidity();
  int humidityOUT = dht.getHumidity();

  //Track historical high and low readings
   
  if (humidityGLOVE > HIGHREADING)
  {
    HIGHREADING=humidityGLOVE;
  }
  if (humidityGLOVE < LOWREADING)
  {
     LOWREADING=humidityGLOVE;
  }

  //Trigger events based on humidity level
  if (humidityGLOVE >= 85)
  {
   Serial.print("Glove Humidity: ");
   Serial.print(humidityGLOVE);
   Serial.println("%\r\n");
   Serial.println("Fan on");
   analogWrite(TIP120pin, 255);          // By changing values from 0 to 255 you can control motor speed
   if ( FANPREV != 1 && DRYMODE != 1)
   {
     startTime = millis();               //Store the fan start time once per drying cycle (per high RH)
   }
   FANPREV = 1;                          //Record that fan has run (used in LCD_DISPLAY()
   DRYMODE=1;                            //Signify drymode; fan should be running
   }
   else if (humidityGLOVE <= 80)
   {
    Serial.print("Glove Humidity: ");
    Serial.print(humidityGLOVE);
    Serial.print("%");
    Serial.print("\tRoom Humidity: ");
    Serial.print(humidityOUT);
    Serial.println("%\r\n");
    Serial.println("Fan off");
    while(ONCE < 1 && DRYMODE == 1 )
    {
      ONCE++;
      FANCONTINUOUS();
    }
    analogWrite(TIP120pin, 0); // Fan off
    DRYMODE=0;
   }
      delay(1000);
} 

//FUNCTIONS

void FANCONTINUOUS()
{
int count=0;
float humidityGLOVE = TH02.ReadHumidity();
  
while(count < 300){
    if (BUTTONVAL == LOW)               // if button has been pushed
      {
        LCD_DISPLAY();
      }
  analogWrite(TIP120pin, 255);
  Serial.print("Humidity: ");
  Serial.print(humidityGLOVE);
  Serial.println("%\r\n");
  Serial.println("Fan on CONTINUOUS");
  delay(1000);
  count++;
  }
}

void LCD_DISPLAY()
{
    float humidityGLOVE = TH02.ReadHumidity();
    display.setCursor(0,0);
    display.print("Humid:");
    display.setCursor(50,0);
    if (humidityGLOVE >= 85)
    {
      display.setTextColor(RED); 
    }
    display.print(humidityGLOVE);
    display.setTextColor(WHITE);
    display.setCursor(80,0);
    display.print("%");
    display.setCursor(0,20);
    display.print("Mode:");
    if (DRYMODE == 1 && ONCE == 0)
    {
      display.setCursor(50,20);
      display.print("DRY");
    }
    else if (ONCE > 0)
    {
      display.setCursor(50,20);
      display.print("CONT");
    }
    else if (ONCE == 0 && DRYMODE != 1)
    {
      display.setCursor(50,20);
      display.print("MONITOR");
    }
    delay(1500);
    display.fillScreen(BLACK);
    display.setCursor(0,0);
    display.print("High:");
    display.setCursor(50,0);
    display.print(HIGHREADING);
    display.print("%");
    display.setCursor(0,20);
    display.print("Low:");
    display.setCursor(50,20);
    display.print(LOWREADING);
    display.print("%");
    if (FANPREV == 1)
    {
      display.setCursor(0,40);
      display.print("Time:");
      display.setCursor(50,40);
      elapsedTime = millis() - startTime;
        if ( (int)(elapsedTime / 1000L) > 100 )
        {
          elapsedTime = millis() - startTime;
          display.print( (int)(elapsedTime / 60000L)); //convert to minutes
          display.print(" mins");
        }
    else 
    {
      elapsedTime = millis() - startTime;
      display.print( (int)(elapsedTime / 1000L));
      display.print(" secs");
    }
    }
    delay(1500);
    display.fillScreen(BLACK);
}
