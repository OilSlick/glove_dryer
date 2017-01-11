// #include "TH02_dev.h"        // Needed for Grove TH02 sensor
// #include "Arduino.h"         //(I don't remember why I include this)
// #include "Wire.h"            //Permits I2C comms but TH02_dev.h includes functionality
#include "DHT.h"                //Permits DHT usage
#include "RunningAverage.h"     //Calculates running average for sensor correlation.
#include "LowPower.h"           //Provides sleep/idle/powerdown functions
#include <Adafruit_NeoPixel.h>  //Drives the Neopixel LEDs
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define ledPIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, ledPIN, NEO_GRB + NEO_KHZ800);

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
  //NeoPixels setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
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
    rainbow(20);                          //Display rainbow fade with pixels
    analogWrite(TIP120pin, 255);          //Turn fan on "full" (255 = full)   
  }
  else 
  {
    strip.show(); //All pixels off
    DISPLAYSERIAL();
    Serial.println("Fan off");
    analogWrite(TIP120pin, 0); // Fan off
    DRYMODE=0;
  }
  Serial.println("Power nap...");
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
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

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
