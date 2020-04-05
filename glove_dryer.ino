/*
 *Board: Arduino Genuino Uno
 *Sensors: DHT11
 *Button: Adafruit Standalone Toggle Capacitive Touch Sensor Breakout - AT42QT1012
*/

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
bool buttonPressed = 0;                     //track if button was pressed

int humidityOUT = 0;                        //Humidity outside of glove
int humidityGLOVE = 0;                      //Humidity inside of glove
int humidityOUTcorrelated = 0;              //Correlated DHT value
int humidityDIFF = 0;                       //Difference between DHT11 sensors
int humidityBaseline = 0;                   //Used to track current humidity VS when button pushed
char dhtOutStatus;
char dhtGloveStatus;                        

bool FANON = 0;                             //Track fan state
bool DEBUG = 0;                             //Set to "1" to initiate debug routine
bool sensorFail = 0;                        //Set to "1" if either sensor fails
bool OutSensorFail = 0;                     //Set to 1 if sensor fail
bool GloveSensorFail = 0;                   //Set to 1 if sensor fail
volatile unsigned long lastOnTime;          //Record the time fan turned on
int long onDuration = (1800000);            //Time in millis to leave fan on (1800000 = 30m)

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

    if ( dhtGlove.getStatusString() == "OK" )
  {
    Serial.println("glove sensor OK");
  }
  else 
  {
    sensorFail = 1;
    GloveSensorFail = 1;
    Serial.println("glove sensor FAIL");
    LEDblinkRed();
  }
  if ( dhtOut.getStatusString() == "OK" )
  {
    Serial.println("room sensor OK");
  }
  else 
  {
    sensorFail = 1;
    OutSensorFail = 1;
    Serial.println("room sensor FAIL");
    LEDblinkRed();
  }
    Serial.println("Ready");
}
 
void loop()
{ 
  if ( dhtGlove.getStatusString() == "TIMEOUT" )
  {
    sensorFail = 1;
    GloveSensorFail = 1;
  }
  if ( dhtOut.getStatusString() == "TIMEOUT" )
  {
    sensorFail = 1;
    OutSensorFail = 1;
  }
  if ( sensorFail == 1 && FANON == 0 )
  {
    LEDfadeYellow();
  }
  if ( digitalRead(buttonpin) == HIGH && FANON == 0 && buttonPressed == 0 ) //detect state change
  {
    buttonPressed = 1;
    
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanon();
    if ( lastOnTime == 0 )
    {
      lastOnTime = millis();
    }
    if ( humidityBaseline == 0 ) 
    {
      humidityBaseline = humidityOUT;
    }
  }
  if ( digitalRead(buttonpin) == LOW && buttonPressed == 1 ) //detect state change
  {
    buttonPressed = 0;
    /*if ( FANON == 1 )
    {
      turnfanoff();
    } */
  }
  if ( FANON == 1 )
  {
    LEDfadeBlue();
    readSensors();
   if ( millis() >= (lastOnTime + onDuration) )
    {
      if ( sensorFail == 0 )
      {
        //Leave fans on but reset timer and re-evaluate status
        lastOnTime = millis();
      }
      else 
      {
        if ( humidityBaseline > humidityOUT ) //if the room humidity is dropping since turning on fan
        {
          humidityBaseline = humidityOUT;
          lastOnTime = millis();            //extend ontime by onDuration
          Serial.println("Resetting timer. Extending fantime");
        }
        else if ( humidityOUT < 60 )  //if the button pressed and AC is on (low RH) keep fan on
        {
          lastOnTime = millis();
        }
        else 
        {
          turnfanoff(); // Fan off
          LEDfadeGreen();
        }
      }                        
    }
  }
  else 
  {
    turnfanoff(); // Fan off
    if ( sensorFail != 1 )
    {
      LEDfadeGreen();
    }
    if ( sensorFail == 1 )
    {
      LEDfadeYellow();
    }
    readSensors();
  }

  //Trigger events based on difference in humidity levels
  if ( humidityGLOVE >= (humidityOUTcorrelated + 3) && sensorFail != 1 )  
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanon();
    lastOnTime = millis();
  }
  else if ( humidityGLOVE <= (humidityOUTcorrelated + 2) && FANON == 1 && (digitalRead(buttonpin) == LOW) && sensorFail != 1 )
  //if the two sensors are close enough, the fan is already on, and the button isn't active, turn fan off
  //if the button is on, over-ride sensors and stay on. 
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanoff();
    lastOnTime = 0;                         //Reset timer
  }
  /*else if ( FANON == 1 && (digitalRead(buttonpin) == LOW) && sensorFail == 1 )
  {
    if ( Serial )
    {
      DISPLAYSERIAL();
    }
    turnfanoff();
    lastOnTime = 0;                         //Reset timer
    humidityBaseline = 0;                   //Reset baseline
  }*/
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
  FANON = 1;
}
void turnfanoff()
{
  analogWrite(TIP120pin, 0);
  FANON = 0;
  lastOnTime = 0;                         //Reset timer
  humidityBaseline = 0;                   //Reset baseline
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
  Serial.print("sensorFail: ");
  Serial.println( sensorFail );
  Serial.print("humidityBaseline: ");
  Serial.println(humidityBaseline);
  Serial.print("lastOnTime: ");
  Serial.println(lastOnTime);
  Serial.print("Current millis: ");
  Serial.println( millis() );
  if ( FANON == 1)
  {
    Serial.print("Next eval: ");
    Serial.println( lastOnTime + onDuration );
  }
}

void correlateSensors()
{
  while (millis() < 11000)
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

void LEDfadeGreen()
{
      //Fade in
    for (int fadeValue = 0 ; fadeValue <= 16; fadeValue += 1) {
    // sets the value (range from 0 to 255):
    setColor(0, fadeValue, 0); 
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade out
  for (int fadeValue = 16 ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from 0 to 255):
    setColor(0, fadeValue, 0);  
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}

void LEDfadeBlue()
{
      //Fade in
    for (int fadeValue = 0 ; fadeValue <= 16; fadeValue += 1) {
    // sets the value (range from 0 to 255):
    setColor(0, 0, fadeValue); 
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade out
  for (int fadeValue = 16 ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from 0 to 255):
    setColor(0, 0, fadeValue); 
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}

void LEDfadeYellow()
{
      //Fade in
    for (int fadeValue = 0 ; fadeValue <= 16; fadeValue += 1) {
    // sets the value (range from 0 to 255):
    setColor(fadeValue + 32, fadeValue, 0); 
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

  //Fade out
  for (int fadeValue = 16 ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from 0 to 255):
    setColor(fadeValue, fadeValue, 0); 
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
}
