#include "DHT.h"                            //Permits DHT usage
#include "RunningAverage.h"                 //Calculates running average for sensor correlation.
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Running average vars
RunningAverage myRA(10);

//For RGB LED
const int redLED = 9;
const int greenLED = 10;
const int blueLED = 11;
const int TIP120pin = 5;                    //Base pin of TIP120 transistor

int humidityOUT = 0;                        //Humidity outside of glove
int humidityOUTcorrelated = 0;              //Correlated DHT value
int humidityGLOVE = 0;                      //Humidity inside of glove
int humidityDIFF = 0;                       //Difference between DHT11 sensors

bool FANON = 0;                             //Track fan state
volatile unsigned long lastOnTime;          //Record the time fan turned on
int long onDuration = 120000;               //Time in millis to leave fan on

DHT dhtGlove;
DHT dhtOut;
  
#include <SPI.h>

void setup()
{
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
  if ( FANON == 1 )
  {
    setColor(0, 0, 32);  // blue
    while (millis() <= (lastOnTime + onDuration) ); 
      {
        analogWrite(TIP120pin, 255);        //Turn fan on "full" (255 = full)
      }
  }
  else if ( FANON == 0 )
  {
    setColor(0, 32, 0);                     // LED Green
    lastOnTime = 0;                         //Reset timer
  }
  
  humidityGLOVE = dhtGlove.getHumidity();
  humidityOUT = dhtOut.getHumidity();
  humidityDIFF = humidityGLOVE - humidityOUT;
  humidityOUTcorrelated = humidityOUT + myRA.getAverage();
  
  //Trigger events based on difference in humidity levels
  if (humidityGLOVE >= (humidityOUTcorrelated + 2) && FANON == 0)  
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
      Serial.println("Fan on");
    }
    analogWrite(TIP120pin, 255);            //Turn fan on "full" (255 = full)  
    FANON = 1;
  }
  else 
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
      Serial.println("Fan off");
    }
    analogWrite(TIP120pin, 0); // Fan off
    FANON = 0;
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
  Serial.println(myRA.getAverage(), 3);
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
  setColor(64, 32, 0);  // blue
  delay(50);
  setColor(0, 0, 0);      // no LED
  delay(50);
  setColor(64, 32, 0);  // blue
  delay(50);
  setColor(0, 0, 0);      // no LED

  myRA.addValue(humidityDIFF);        //Add diff to running average

  delay(1000);
  }
}

void setColor(int red, int green, int blue)
{
  analogWrite(redLED, 255-red);
  analogWrite(greenLED, 255-green);
  analogWrite(blueLED, 255-blue);  
}
