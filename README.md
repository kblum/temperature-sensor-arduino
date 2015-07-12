# Arduino Temperature Sensor

This project is an Arduino temperature sensor using a [DS18B20](http://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html) digital thermometer (1-Wire interface) and an [Adafruit HUZZAH CC3000 WiFi Shield](http://www.adafruit.com/products/1491).

Hardware instructions are based on the [Arduino Uno](https://www.arduino.cc/en/Main/arduinoBoardUno) R3 board. Software instructions are based on version 1.6.5 of the [Arduino IDE](https://www.arduino.cc/en/Main/Software).


## Hardware

The 1-Wire sensor bus should be connected to digital I/O pin 2.

Instructions on connecting the CC3000 WiFi Shield can be found in the [tutorial](https://learn.adafruit.com/adafruit-cc3000-wifi).


## Software Installation

Go to "Manage Libraries" in the Arduino IDE and install the following libraries:

* [OneWire](http://www.pjrc.com/teensy/td_libs_OneWire.html) - project was developed against version 2.3.0.
* [MAX](MAX31850_DallasTemp) - project was developed against version 1.0.0.

Install the CC3000 WiFi Shield library by following the [instructions in the tutorial](https://learn.adafruit.com/adafruit-cc3000-wifi/cc3000-library-software).


## Configuration

Create a `config.h` file and populate with the following settings (substituting in appropriate values):

```
// the pin that the 1-Wire sensor bus is connected to
// multiple sensors can be connected to the same 1-Wire bus
#define ONE_WIRE_PIN 2

// SSID and password cannot be longer than 32 characters
#define WLAN_SSID "SSID"
#define WLAN_PASS "PASSWORD"
// security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY WLAN_SEC_WPA2

// API endpoint
#define API_HOST "HOST"
#define API_ENDPOINT "/api"
#define API_PORT 80
// empty string for no auth or base64 encoded auth header value
#define API_AUTH_VALUE ""

// set to true to perform DNS lookup from API_HOST
// false to use fixed IP address
#define PERFORM_DNS_LOOKUP false

// components of fixed IP address
// only required if PERFORM_DNS_LOOKUP is false
#define API_IP1 127
#define API_IP2 0
#define API_IP3 0
#define API_IP4 1
```

The configuration file is not under version control.


## API

The temperature readings are sent to a remote server through an HTTP/1.1 POST request. The data is sent in JSON format.

The following headers are included in the request:

* `Host`
* `Authorization` - HTTP Basic authentication (if configured)
* `User-Agent` - `Arduino/1.0`
* `Content-Type` - `application/json`
* `Connection` - `close`
* `Content-Length`

Example of request body:

    { "readings": { "0x28ff123456789abc":14.44, "0x28ff123456789abd":14.38 } }

The unique 64-bit serial code for each device (in hexadecimal format) is mapped to the temperature reading from the device.
