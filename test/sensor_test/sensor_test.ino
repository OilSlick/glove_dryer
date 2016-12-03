// DHT readings made simple by Mark Ruys:
// https://github.com/markruys/arduino-DHT
// Running averages made simple by Rob Tillaart:
// https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage

#include "DHT.h"
#include <TH02_dev.h>
#include "RunningAverage.h"

RunningAverage myRA(10);
int samples = 0;
int humidity = 0;

DHT dht;

void setup()
{
  Serial.begin(9600);

  dht.setup(A0);
  
  //Temp+humid requirements
  Serial.println("****THO2 power up delay, letting voltage settle****\n");
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n"); 

  myRA.clear(); // explicitly start RA clean
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  int humidityDHT = dht.getHumidity();
  int humidityTOP = TH02.ReadHumidity();
  int humidityDIFF = humidityTOP - humidityDHT;
  
  Serial.print("DHT humdity = ");
  Serial.print(humidityDHT);

  Serial.print("\t Top Humidity: ");
  Serial.print(humidityTOP);

  Serial.print("\t Difference: ");
  Serial.print(humidityDIFF);

  myRA.addValue(humidityDIFF);
  samples++;
  Serial.print("\t Running Average: ");
  Serial.print(myRA.getAverage(), 3);
  Serial.print("\t Samples: ");
  Serial.println(samples);

  Serial.print("Minimum: ");
  Serial.print(myRA.getMin(), 3);
  Serial.print("\t Maximum: ");
  Serial.println(myRA.getMax(), 3);

  if (samples == 300)
  {
    samples = 0;
    myRA.clear();
    Serial.println();
  }
  delay(10);

}
