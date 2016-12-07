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

### To-Do
* [ ] Switch FANCONTINUOUS to use millis instead of delay
* [ ] Have fan-to-PVC pipe bracket printed 3D
* [ ] Enable power-save for Arduino
* [ ] Put LCD on PWM pin and power up/down as needed
 * [ ] Utilize IfSerial
* [ ] Power Arduino with [PowerBoost 1000 Charger](https://www.adafruit.com/products/2465) (currently on AAA's)
 * [ ] Recharge Arduino from wall wart used to power fans
* [ ] Have Arduino case and pipe-supports printed 3D


*The sensor I am using outside of the gloves is a DHT11 which is [known to have calibration issues](https://forum.arduino.cc/index.php?topic=96470.0). This project doesn't require hyper-accurate RH readings so I use a rather clumsy method of calibrating the two using a running average of the difference.