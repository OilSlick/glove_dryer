
#include "TH02_dev.h"
#include "Arduino.h"
#include "Wire.h"
#include "DHT.h" 
#include "RunningAverage.h"

//Running average vars
RunningAverage myRA(10);
int samples = 0;                //Count number of samples taken
int cycle = 0;                  //Count number of cycles between RA resets
float previousRA = 0;           //Track previous running average (RA)
int humidityOUT = 0;            //Humidity outside of glove
int humidityOUTcorrelated = 0;  //Correlated DHT value
int humidityGLOVE = 0;          //Humidity inside of glove
int humidityDIFF = 0;           //Difference betwee TH02 and DHT11 sensors


const int TIP120pin = 5;        //Base pin of TIP120 transistor
const int ButtonPWR = 6;        //Provide 5v to button circuit
const int inPin = 7;            //Read button circuit status

int BUTTONVAL = 0;              //Store button circuit status as value
int DRYMODE=0;                  //Determine monitor or dry mode (fan on or off)
int LOWREADING=200;             //Track historical low RH
int HIGHREADING=0;              //Track historical high RH
long startTime ;                //Track fan runtime
long elapsedTime ;              //Track fan runtime
int FANPREV = 0;                //Store if fan has ever run

DHT dht;
  
//LCD requirements:

// Pin assignments for LCD display
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
  Serial.println("THO2 power up delay, letting voltage settle");
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  Serial.println("TH02 Ready"); 

  //TIP120 Transistor base pin as OUTPUT
  pinMode(TIP120pin, OUTPUT);

  //LCD Display requirements
  display.begin();
  display.fillScreen(BLACK);
  display.setCursor(0,0);
  display.print("Glove Dryer V.1");
  delay(1000);
  display.fillScreen(BLACK);

  myRA.clear(); // explicitly start RA clean
  delay(dht.getMinimumSamplingPeriod()); //play nice with DHT sensor

  Serial.println("Beginning sensor correlation");

  while (millis() < 10000)
  {
  Serial.print("Millis: ");
  Serial.println(millis());
  humidityGLOVE = TH02.ReadHumidity();
  Serial.print("TH02: ");
  Serial.print(humidityGLOVE);
  humidityOUT = dht.getHumidity();
  Serial.print("\t DHT: ");
  Serial.print(humidityOUT);
  humidityDIFF = humidityGLOVE - humidityOUT;
  Serial.print("\t Diff: ");
  Serial.println(humidityDIFF);

  myRA.addValue(humidityDIFF);        //Add diff to running average
  samples++;

  delay(1000);
  }
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

  humidityGLOVE = TH02.ReadHumidity();
  humidityOUT = dht.getHumidity();
  humidityDIFF = TH02.ReadHumidity() - humidityOUT;
  humidityOUTcorrelated = humidityOUT + myRA.getAverage();
  
  //Track historical high and low readings for display on LCD
   
  if (humidityGLOVE > HIGHREADING)
  {
    HIGHREADING=humidityGLOVE;
  }
  if (humidityGLOVE < LOWREADING)
  {
     LOWREADING=humidityGLOVE;
  }

  //Trigger events based on difference in humidity levels
  if (humidityGLOVE >= (humidityOUTcorrelated + 3))  
  {
    DISPLAYSERIAL();
    Serial.println("Fan on");
    analogWrite(TIP120pin, 255);          // Turn fan on "full" (255 = full)
    if ( FANPREV != 1 && DRYMODE != 1)
    {
      startTime = millis();               //Store the fan start time once per drying cycle
    }   
  }
  else 
  {
  DISPLAYSERIAL();
  Serial.println("Fan off");
  analogWrite(TIP120pin, 0); // Fan off
  DRYMODE=0;
  }
  delay(1000);
} 

//FUNCTIONS

void LCD_DISPLAY()
{
    int humidityGLOVE = TH02.ReadHumidity();
    display.setCursor(0,0);
    display.print("Humid:");
    display.setCursor(50,0);
    if (humidityGLOVE >= 85)
    {
      display.setTextColor(RED); 
    }
    display.print(humidityGLOVE);
    display.setTextColor(WHITE);
    display.print("%");
    display.setCursor(0,20);
    display.print("Mode:");
    if (DRYMODE == 1)
    {
      display.setCursor(50,20);
      display.print("DRY");
    }

    else if (DRYMODE != 1)
    {
      display.setCursor(50,20);
      display.print("MONITOR");
    }
    delay(2000);
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

void DISPLAYSERIAL()
{
  Serial.print("Glove Humidity: ");
  Serial.print(humidityGLOVE);
  Serial.print("%");
  Serial.print("\t Raw Room Humidity: ");
  Serial.print(humidityOUT);
  Serial.print("%");
  Serial.print("\t Correlated Room Humidity: ");
  Serial.print(humidityOUTcorrelated);
  Serial.print("\t Diff RA: ");
  Serial.print(myRA.getAverage(), 3);
  Serial.print("\t High: ");
  Serial.print(HIGHREADING);
  Serial.print("\t Low: ");
  Serial.println(LOWREADING);
}

