This project uses an Arduino Uno to read relative humidity from two sensors; one inside a glove and another outside the glove. Based on the difference of the two values, it runs a number of PC fans to move wetter air out of the gloves and drier air into the gloves. 

### Basic Operation

1. Check relative humidity (RH) at both sensors
2. Start a running average (RA) of the difference between the two to correlate*
3. If the correlated values differ by >= 3%, turn on fans for 30 minutes
4. After 30 minutes, shut down the fans and recheck humidity.
 
### To-Do
* [x] ~~Switch FANCONTINUOUS to use millis instead of delay~~ (decided against FANCONTINUOUS)
* [x] Have fan-to-PVC pipe bracket printed 3D
* [ ] ~~Enable power-save for Arduino~~ (June 1, 2017: removed power-saving code)
* [ ] ~~Put LCD on PWM pin and power up/down as needed~~ (removed LCD)
 * [x] Utilize IfSerial
* [ ] ~~Power Arduino with [PowerBoost 1000 Charger](https://www.adafruit.com/products/2465)~~ (used OKI-78SR with 12v wall plug)
* [x] ~~Have Arduino case~~ and pipe-supports printed 3D (test print proved the box would be prohibitively expensive. Pipe supports work great)
* [x] Complete box (~~using old moo card box~~ Used plastic box from Muji)
* [x] Final assembly of fan posts
* [ ] Add a momentary switch for manual correlation


*I am using two DHT11 humidity sensors and they tend to be roughly 2 percentage points off from one another, so I do a series of tests to see what the difference is between the two then apply that value to create a correlated value to compensate for the difference.