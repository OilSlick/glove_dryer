# sensor_test

The first temp/humi sensor I bought was the [Grove - Temperature&Humidity Sensor (High-Accuracy &Mini) v1.0](http://wiki.seeed.cc/Grove-TemptureAndHumidity_Sensor-High-Accuracy_AndMini-v1.0/). Then I decided I wanted a second sensor to read relative humidity outside of the gloves, in order to bring the inside RH down to the outside RH, rather than some arbitrary RH value.fasfda

The second sensor I bought was the cheaper [Grove - Temp&Humi Sensor](https://www.seeedstudio.com/Grove-Temp%26amp%3BHumi-Sensor-p-745.html), which I later found tends to have [calibration issues](https://forum.arduino.cc/index.php?topic=96470.0). 

So I put together this sketch to see how far apart the two sensors are. Since I don't need hyper-accurate readings for this project, the thought is to keep a running average of the difference in RH and calibrate the two programatically using the running average. 
