# glove_dryer
An Arduino-based boxing glove dryer.

This project uses an Arduino Uno to read relative humidity from two sensors; one inside a glove and another outside the glove. Based on the difference of the two values, it runs a number of PC fans to move wetter air out of the gloves and drier air into the gloves. 

### Basic Operation

1. Check relative humidity (RH) at both sensors
2. Start a running average (RA) of the difference between the two to calibrate*
3. If the calibrated values differ by >= 3%, turn on fans, stop active calibration
4. Once RH values converge to within <= 3%, run fans 5 more minutes to ensure stable RH inside gloves
5. Gloves are dry, start monitoring again.

**As of December 4, 2016 this project is still being developed.** While the Arduino sketch does provide full functionality, I am only currently testing with 1/2 of the solution; an exhaust fan inside the glove. The goal is to have another fan (per glove) that blows outside air into the glove so that there is a push/pull type action on air inside the glove. 

###Update January 13, 2017

The Arduino Sketch is mostly done. There is some lag in detecting the initial glove placement. This is caused by powering down the peripherals to save power. I think the solution is to power down for 8 seconds then leave it powered up for 3 seconds to give the DHT11 sensors enough time to register the humid glove. 

### To-Do
* [x] ~~Switch FANCONTINUOUS to use millis instead of delay~~ (decided against FANCONTINUOUS)
* [x] Have fan-to-PVC pipe bracket printed 3D
* [x] Enable power-save for Arduino
* [x] ~~Put LCD on PWM pin and power up/down as needed~~ (removed LCD)
 * [x] Utilize IfSerial
* [x] ~~Power Arduino with [PowerBoost 1000 Charger](https://www.adafruit.com/products/2465)~~ (will use 7805 regulator with wall wart)
* [ ] Have Arduino case and pipe-supports printed 3D


*The sensor I am using outside of the gloves is a DHT11 which is [known to have calibration issues](https://forum.arduino.cc/index.php?topic=96470.0). This project doesn't require hyper-accurate RH readings so I use a rather clumsy method of calibrating the two using a running average of the difference.