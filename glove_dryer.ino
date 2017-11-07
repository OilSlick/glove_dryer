#include "DHT.h"                            //Permits DHT usage
#include "RunningAverage.h"                 //Calculates running average for sensor correlation.
#include <SPI.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Running average vars
RunningAverage myRA(10);

//For RGB LED
const int TIP120pin = 5;                    //Base pin of TIP120 transistor
const int redLED = 9;
const int greenLED = 10;
const int blueLED = 11;
const int buttonpin = 12;                   //Output of capacitive touch switch

int humidityOUT = 0;                        //Humidity outside of glove
int humidityOUTcorrelated = 0;              //Correlated DHT value
int humidityGLOVE = 0;                      //Humidity inside of glove
int humidityDIFF = 0;                       //Difference between DHT11 sensors
char dhtOutStatus;
char dhtGloveStatus;                        

bool FANON = 0;                             //Track fan state
bool DEBUG = 0;                             //Set to "1" to initiate debug routine
volatile unsigned long lastOnTime;          //Record the time fan turned on
// int long onDuration = (1800000);            //Time in millis to leave fan on
int long onDuration = (5000);               //For debugging

DHT dhtGlove;
DHT dhtOut;

void setup()
{
  pinMode(TIP120pin, OUTPUT);
  pinMode(buttonpin, INPUT);
  
  //For RGB LED
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  analogWrite(redLED, 255);
  analogWrite(greenLED, 255);
  analogWrite(blueLED, 255);
  
  //DHT sensor setup
  dhtGlove.setup(A0);
  dhtOut.setup(A1);

  Serial.begin(9600);                         // start serial for output

  myRA.clear(); // explicitly start RA clean
  
  Serial.println("Beginning sensor correlation");
  correlateSensors();
  Serial.println("Ready");
}
 
void loop()
{ 
  if ( digitalRead(buttonpin) == HIGH )
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanon();
    lastOnTime = millis(); 
  }
  if ( DEBUG == 1)
  {
     LEDblinkRed();
     turnfanon();
     delay(2000);
     LEDblinkRed();
     turnfanoff();
     delay(2000);     
  }
  if ( FANON == 1 )
  {
    fadeBlue();
   if ( millis() >= (lastOnTime + onDuration) )
    {
      //Leave fans on but reset timer and re-evaluate status
      lastOnTime = 0;                         
    }
    readSensors();
  }
  else 
  {
    analogWrite(TIP120pin, 0); // Fan off
    fadeGreen();

    lastOnTime = 0;                         //Reset timer
    readSensors();
  }
  
  //Trigger events based on difference in humidity levels
  if (humidityGLOVE >= (humidityOUTcorrelated + 3) )  
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanon();
    lastOnTime = millis();
  }
  else if ( humidityGLOVE <= (humidityOUTcorrelated + 2) && FANON == 1 )
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanoff();
    lastOnTime = 0;                         //Reset timer
  }
  else
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
  }
  delay(dhtGlove.getMinimumSamplingPeriod()); //play nice with DHT sensor
} 

//FUNCTIONS

void turnfanon()
{
  analogWrite(TIP120pin, 255);
  //setColor(0, 0, 32);                     // LED blue
  FANON = 1;
}
void turnfanoff()
{
  analogWrite(TIP120pin, 0);
  //setColor(0, 255, 0);                     // LED Green
  FANON = 0;
}
void LEDblinkRed()
{
  setColor(255, 0, 0);  // Red
  delay(50);
  setColor(0, 0, 0);      // no LED
  delay(50);
  setColor(255, 0, 0);  // Red
  delay(50);
  setColor(0, 0, 0);      // no LED
}

void readSensors()
{
  humidityGLOVE = dhtGlove.getHumidity();
  humidityOUT = dhtOut.getHumidity();
  humidityDIFF = humidityGLOVE - humidityOUT;
  humidityOUTcorrelated = humidityOUT + myRA.getAverage();
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
  Serial.print("\t Running Average: ");
  Serial.println(myRA.getAverage(), 3);
  Serial.print("DHT-Glove Status: ");
  Serial.print(dhtGlove.getStatusString());
  Serial.print("\t DHT-Out Status: ");
  Serial.print(dhtOut.getStatusString());
  Serial.print("\t DHT Delay ");
  Serial.println(dhtGlove.getMinimumSamplingPeriod());
  Serial.print("Fan Status: ");
  if ( FANON == 1)
  {
    Serial.println("ON");
  }
  else 
  {
    Serial.println("OFF");
  }
}

void correlateSensors()
{
  while (millis() < 10000)
  {
    humidityGLOVE = dhtGlove.getHumidity();
    humidityOUT = dhtOut.getHumidity();
    humidityDIFF = humidityGLOVE - humidityOUT;
    humidityOUTcorrelated = humidityOUT + myRA.getAverage();

  if ( Serial ) {
    Serial.print("Millis: ");
    Serial.println(millis());
    Serial.print("DHT Glove: ");
    Serial.print(humidityGLOVE);
    Serial.print("\t DHT Out: ");
    Serial.print(humidityOUT);
    Serial.print("\t Diff: ");
    Serial.println(humidityDIFF);
    Serial.print("Correlated Room Humidity: ");
    Serial.println(humidityOUTcorrelated);
  }

  //Blink LED
  setColor(64, 32, 0);  // Yellow
  delay(50);
  setColor(0, 0, 0);      // no LED
  delay(50);
  setColor(64, 32, 0);  // Yellow
  delay(50);
  setColor(0, 0, 0);      // no LED

  myRA.addValue(humidityDIFF);        //Add diff to running average

  delay(dhtGlove.getMinimumSamplingPeriod()); //play nice with DHT sensor
  }
}

void setColor(int red, int green, int blue)
{
  analogWrite(redLED, 255-red);
  analogWrite(greenLED, 255-green);
  analogWrite(blueLED, 255-blue);  
}

void blinkError()
{
  //Fade red in
    for (int fadeValue = 0 ; fadeValue <= 64; fadeValue += 2) {
    // sets the value (range from 0 to 255):
    setColor(fadeValue, 0, 0);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade red out
  for (int fadeValue = 64 ; fadeValue >= 0; fadeValue -= 2) {
    // sets the value (range from 0 to 255):
    setColor(fadeValue, 0, 0);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}

void fadeGreen()
{
      //Fade green in
    for (int fadeValue = 0 ; fadeValue <= 16; fadeValue += 1) {
    // sets the value (range from 0 to 255):
    setColor(0, fadeValue, 0);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade green out
  for (int fadeValue = 16 ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from 0 to 255):
    setColor(0, fadeValue, 0);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}

void fadeBlue()
{
      //Fade green in
    for (int fadeValue = 0 ; fadeValue <= 16; fadeValue += 1) {
    // sets the value (range from 0 to 255):
    setColor(0, 0, fadeValue);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade green out
  for (int fadeValue = 16 ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from 0 to 255):
    setColor(0, 0, fadeValue);  // red
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}

