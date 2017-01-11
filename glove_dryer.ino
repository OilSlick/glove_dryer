// #include "TH02_dev.h"      // Needed for Grove TH02 sensor
// #include "Arduino.h"       //(I don't remember why I include this)
// #include "Wire.h"          //Permits I2C comms but TH02_dev.h includes functionality
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
int humidityDIFF = 0;           //Difference between TH02 and DHT11 sensors

const int TIP120pin = 5;        //Base pin of TIP120 transistor

int DRYMODE=0;                  //Determine monitor or dry mode (fan on or off)
int LOWREADING=200;             //Track historical low RH
int HIGHREADING=0;              //Track historical high RH
long startTime ;                //Track fan runtime
long elapsedTime ;              //Track fan runtime
int FANPREV = 0;                //Store if fan has ever run

DHT dhtGlove;
DHT dhtOut;
  
#include <SPI.h>

void setup()
{  
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

  while (millis() < 10000)
  {
  Serial.print("Millis: ");
  Serial.println(millis());
  humidityGLOVE = dhtGlove.getHumidity();
  Serial.print("TH02: ");
  Serial.print(humidityGLOVE);
  humidityOUT = dhtOut.getHumidity();
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
  if (humidityGLOVE >= (humidityOUTcorrelated + 5))  
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
