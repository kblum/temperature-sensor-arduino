/**
 * Arduino Temperature Sensor
 */

#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"

/*****
 * Values expected in config.h:

 * // the PIN that the 1-Wire sensor bus is connected to
 * // multiple sensors can be connected to the same 1-Wire bus
 * #define ONE_WIRE_PIN 2
 ****/

// initialise OneWire library
OneWire oneWire(ONE_WIRE_PIN);

// initialise temperature library
DallasTemperature sensors(&oneWire);

byte deviceCount;
byte deviceAddress[8];
byte deviceIndex;

// setup code, runs once
void setup() {
  // start serial port
  Serial.begin(115200);

  Serial.println(F("Arduino Temperature Sensor"));
  Serial.println(F("--------------------------"));
  Serial.println();

  // initialise temperature sensors
  sensorInit();

  // get readings from temperature sensors
  readSensors();

  Serial.println();
}

// main loop, runs repeatedly
void loop() {
  // no nothing
  delay(1000);
}

void sensorInit() {
  /**
   * Initialise all sensors on 1-Wire bus.
   */
   
  Serial.print(F("Sensor bus on PIN "));
  Serial.println(ONE_WIRE_PIN, DEC);

  // start temperature library
  sensors.begin();
  
  deviceCount = sensors.getDeviceCount();
  Serial.print(F("Number of devices on bus: "));
  Serial.println(deviceCount, DEC);

  // report parasite power requirements
  Serial.print(F("Parasite power: ")); 
  if (sensors.isParasitePowerMode()) {
    Serial.println(F("on"));
  }
  else {
    Serial.println(F("off"));
  }
}

void printAddress(byte address[8]) {
  /**
   * Print the address of a device in hexadecimal format to the serial port.
   */
   
  Serial.print(F("{ "));
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    // zero-pad the address if necessary
    if (address[i] < 16) {
      Serial.print(F("0"));
    }
    Serial.print(address[i], HEX);
    // seperate each octet with a comma
    if (i < 7) {
      Serial.print(F(", "));
    }
  }
  Serial.print(F(" }"));
}

void readSensors() {
  /**
   * Get temperature reading from each sensor.
   * 
   * The address of each device is a unique 64-bit code stored in ROM (reference: DS18B20 datasheet).
   * The least significant 8-bits contains the 1-Wire family code (28h for the DS18B20).
   * The next 48 bits contain a unique serial number.
   * The most significant 8-bits contain a CRC calculated from the first 56 bits of the ROM code.
   */
   
  // call sensors.requestTemperatures() to issue a global temperature request to all devices in the bus
  Serial.println(F("Requesting temperatures..."));
  sensors.requestTemperatures();

  for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
    if (sensors.getAddress(deviceAddress, deviceIndex)) {
      float temperature = sensors.getTempC(deviceAddress);
      Serial.print(F("Device "));
      Serial.print(deviceIndex);
      Serial.print(F("; address: "));
      printAddress(deviceAddress);
      Serial.print(F("; temperature: "));
      Serial.println(temperature);
    } else {
      Serial.println(F("Unable to read from device"));
    }
  } 
}

