
#include "TH02_dev.h"
#include "Arduino.h"
#include "Wire.h"
#include "DHT.h" 
#include "RunningAverage.h"

//Running average vars
RunningAverage myRA(10);
int samples = 0;            //Count number of samples taken
int cycle = 0;              //Count number of cycles between RA resets
float previousRA = 0;       //Track previous running average (RA)
int humidityOUTcalibrated = 0; //Calibrated DHT value

const int TIP120pin = 5;    //Base pin of TIP120 transistor
const int ButtonPWR = 6;    //Provide 5v to button circuit
const int inPin = 7;        //Read button circuit status
int BUTTONVAL = 0;          //Store button circuit status as value
int DRYMODE=0;              //Determine monitor or dry mode (fan on or off)
int ONCE=0;                 //Ensure FANCONTINUOUS() only runs once per drying cycle (per high humidity)
int LOWREADING=200;       //Track historical low RH
int HIGHREADING=0;        //Track historical high RH
long startTime ;            //Track fan runtime
long elapsedTime ;          //Track fan runtime
int FANPREV = 0;            //Store if fan has ever run

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

  myRA.clear(); // explicitly start RA clean
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
  
  int humidityGLOVE = TH02.ReadHumidity();
  int humidityOUT = dht.getHumidity();

  //Calibrate DHT sensor (humidityOUT)
  if (cycle < 1)                        //on first RA cycle, use current running average
  {
   humidityOUTcalibrated = humidityOUT + myRA.getAverage();
   Serial.print("Calibrated DHT: ");
   Serial.print(humidityOUTcalibrated);
   Serial.println("%");
  }
  else if (cycle >= 1 && samples <= 149) //Assuming before 150 RA samples, previous RA is more accurate
  {
   humidityOUTcalibrated = humidityOUT + previousRA;
   Serial.print("Calibrated DHT: ");
   Serial.print(humidityOUTcalibrated);
   Serial.println("%");
  }
  else                                   //With 150+ RA samples, use current RA
  {
   humidityOUTcalibrated = humidityOUT + myRA.getAverage();
   Serial.print("Calibrated DHT: ");
   Serial.print(humidityOUTcalibrated);
   Serial.println("%"); 
  }
  
  //Track historical high and low readings
   
  if (humidityGLOVE > HIGHREADING)
  {
    HIGHREADING=humidityGLOVE;
  }
  if (humidityGLOVE < LOWREADING)
  {
     LOWREADING=humidityGLOVE;
  }

  //Trigger events based on difference in humidity levels
  if (humidityGLOVE >= (humidityOUTcalibrated + 3) && samples > 1)  //If gloves are 3+% more humid, commence drying
  {
   Serial.print("Glove Humidity: ");
   Serial.print(humidityGLOVE);
   Serial.println("%\r\n");
   Serial.println("Fan on");
   analogWrite(TIP120pin, 255);          // Turn fan on "full" (255 = full)
   if ( FANPREV != 1 && DRYMODE != 1)
   {
     startTime = millis();               //Store the fan start time once per drying cycle
   }
   FANPREV = 1;                          //Record that fan has run (used in LCD_DISPLAY())
   DRYMODE=1;                            //Signify drymode; fan should be running
   }
   else 
   {
    int humidityDIFF = TH02.ReadHumidity() - dht.getHumidity();
    myRA.addValue(humidityDIFF);
    samples++;

    if (samples == 300)                    //To preserve memory, only run for X samples
    {
      previousRA=myRA.getAverage();       //Store previous RA
      cycle++;
      samples = 0;
      myRA.clear();
      Serial.print("Cycles: ");
      Serial.print(cycle);
    }
    Serial.print("Glove Humidity: ");
    Serial.print(humidityGLOVE);
    Serial.print("%");
    Serial.print("\t Room Humidity: ");
    Serial.print(humidityOUT);
    Serial.print("%");
    Serial.print("\t Diff RA: ");
    Serial.println(myRA.getAverage(), 3);
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
int humidityGLOVE = TH02.ReadHumidity();
  
while(count < 300){
    if (BUTTONVAL == LOW)               // if button has been pushed
      {
        LCD_DISPLAY();
      }
  analogWrite(TIP120pin, 255);
  Serial.print("Glove Humidity: ");
  Serial.print(humidityGLOVE);
  Serial.print("%");
  Serial.print("\t Room Humidity: ");
  Serial.print(humidityOUTcalibrated);
  Serial.print("%");
  Serial.print("\t Diff RA: ");
  Serial.println(myRA.getAverage(), 3);
  Serial.println("Fan on CONTINUOUS");
  delay(1000);
  count++;
  }
  if (count >= 300)
  {
    ONCE = 0;                             // reset ONCE
  }
  count = 0;                            // reset count
}

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
