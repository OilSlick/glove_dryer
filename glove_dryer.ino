#include "DHT.h"                //Permits DHT usage
#include "RunningAverage.h"     //Calculates running average for sensor correlation.
#include "LowPower.h"           //Provides sleep/idle/powerdown functions
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Running average vars
RunningAverage myRA(10);
int samples = 0;                //Count number of RA samples taken
float previousRA = 0;           //Track previous running average (RA)

//For RGB LED
const int redLED = 9;
const int greenLED = 10;
const int blueLED = 11;

int humidityOUT = 0;            //Humidity outside of glove
int humidityOUTcorrelated = 0;  //Correlated DHT value
int humidityGLOVE = 0;          //Humidity inside of glove
int humidityDIFF = 0;           //Difference between DHT11 sensors

const int TIP120pin = 5;        //Base pin of TIP120 transistor

int LOWREADING=200;             //Track historical low RH
int HIGHREADING=0;              //Track historical high RH

DHT dhtGlove;
DHT dhtOut;
  
#include <SPI.h>

void setup()
{
  //For RGB LED
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  
  //DHT sensor setup
  dhtGlove.setup(A0);
  dhtOut.setup(A1);
  
  Serial.begin(9600);             // start serial for output
  
  //TIP120 Transistor base pin as OUTPUT
  pinMode(TIP120pin, OUTPUT);

  myRA.clear(); // explicitly start RA clean
  delay(dhtGlove.getMinimumSamplingPeriod()); //play nice with DHT sensor
  delay(dhtOut.getMinimumSamplingPeriod());   //play nice with DHT sensor

  Serial.println("Beginning sensor correlation");
  correlateSensors();
}
 
void loop()
{ 
  setColor(0, 8, 0);  // green
  
  humidityGLOVE = dhtGlove.getHumidity();
  humidityOUT = dhtOut.getHumidity();
  humidityDIFF = humidityGLOVE - humidityOUT;
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
  if (humidityGLOVE >= (humidityOUTcorrelated + 2))  
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
      Serial.println("Fan on");
    }
    analogWrite(TIP120pin, 255);            //Turn fan on "full" (255 = full)   
  }
  else 
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
      Serial.println("Fan off");
      Serial.println("Power nap...");
    }
    analogWrite(TIP120pin, 0); // Fan off
    //delay(100);                             //Delay the powernap 
    //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    //delay(2000);                            //Delay sensor reading until all powered back on
  }
} 

//FUNCTIONS

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
  setColor(0, 0, 16);  // blue
  delay(50);
  setColor(0, 0, 0);      // no LED
  delay(50);
  setColor(0, 0, 16);  // blue
  delay(50);
  setColor(0, 0, 0);      // no LED

  myRA.addValue(humidityDIFF);        //Add diff to running average
  samples++;

  delay(1000);
  }
}

void setColor(int red, int green, int blue)
{
  analogWrite(redLED, red);
  analogWrite(greenLED, green);
  analogWrite(blueLED, blue);  
}
