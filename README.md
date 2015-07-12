# Arduino Temperature Sensor

This project is an Arduino temperature sensor using a [DS18B20](http://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html) digital thermometer (1-Wire interface).

Hardware instructions are based on the [Arduino Uno](https://www.arduino.cc/en/Main/arduinoBoardUno) R3 board. Software instructions are based on version 1.6.5 of the [Arduino IDE](https://www.arduino.cc/en/Main/Software).


## Hardware

The 1-Wire sensor bus should be connected to digital I/O PIN 2.


## Software Installation

Go to "Manage Libraries" in the Arduino IDE and install the following libraries:

* [OneWire](http://www.pjrc.com/teensy/td_libs_OneWire.html) - project was developed against version 2.3.0.
* [MAX](MAX31850_DallasTemp) - project was developed against version 1.0.0.
