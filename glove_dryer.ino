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

int humidityOUT = 0;                        //Humidity outside of glove
int humidityOUTcorrelated = 0;              //Correlated DHT value
int humidityGLOVE = 0;                      //Humidity inside of glove
int humidityDIFF = 0;                       //Difference between DHT11 sensors
char dhtOutStatus;
char dhtGloveStatus;                        

bool FANON = 0;                             //Track fan state
volatile unsigned long lastOnTime;          //Record the time fan turned on
int long onDuration = (30*60*1000);         //Time in millis to leave fan on

DHT dhtGlove;
DHT dhtOut;

void setup()
{
  pinMode(TIP120pin, OUTPUT);
  
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
  if ( FANON == 1 )
  {
    setColor(0, 0, 32);  // blue
    while (millis() <= (lastOnTime + onDuration) ) 
      {
        analogWrite(TIP120pin, 255);        //Turn fan on "full" (255 = full)
      }
      analogWrite(TIP120pin, 255);          //Turn fan on "full" (255 = full)
      FANON = 0;
      lastOnTime = 0;                       //Reset timer
  }
  else if ( FANON == 0 )
  {
    setColor(0, 32, 0);                     // LED Green
    lastOnTime = 0;                         //Reset timer

    humidityGLOVE = dhtGlove.getHumidity();
    humidityOUT = dhtOut.getHumidity();
    humidityDIFF = humidityGLOVE - humidityOUT;
    humidityOUTcorrelated = humidityOUT + myRA.getAverage();
    }
  
  //Trigger events based on difference in humidity levels
  if (humidityGLOVE >= (humidityOUTcorrelated + 3) && FANON == 0)  
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    analogWrite(TIP120pin, 255);            //Turn fan on "full" (255 = full)  
    FANON = 1;
  }
  else if ( humidityGLOVE < (humidityOUTcorrelated + 2) && FANON == 1 )
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    analogWrite(TIP120pin, 0); // Fan off
    FANON = 0;
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

