# glove_dryer
An Arduino-based boxing glove dryer.

This project uses an Arduino Uno to read relative humidity from two sensors; one inside a glove and another outside the glove. Based on the difference of the two values, it runs a number of PC fans to move wetter air out of the gloves and drier air into the gloves. 

### Basic Operation

1. Check relative humidity (RH) at both sensors
2. Start a running average (RA) of the difference between the two to calibrate*
3. If the calibrated values differ by >= 3%, turn on fans, stop active calibration
4. Once RH values converge to within <= 3%, run fans 5 more minutes to ensure stable RH inside gloves
5. Gloves are dry, start monitoring again.

*The sensor I am using outside of the gloves is a DHT11 which is [known to have calibration issues](https://forum.arduino.cc/index.php?topic=96470.0). This project doesn't require hyper-accurate RH readings so I use a rather clumsy method of calibrating the two using a running average of the difference.