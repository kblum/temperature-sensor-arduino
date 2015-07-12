/**
 * Arduino Temperature Sensor
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#include "config.h"

/*****
 * Values expected in config.h:

 * // the pin that the 1-Wire sensor bus is connected to
 * // multiple sensors can be connected to the same 1-Wire bus
 * #define ONE_WIRE_PIN 2
 * 
 * // SSID and password cannot be longer than 32 characters
 * #define WLAN_SSID "SSID"
 * #define WLAN_PASS "PASSWORD"
 * // security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
 * #define WLAN_SECURITY WLAN_SEC_WPA2
 * 
 * // API endpoint
 * #define API_HOST "HOST"
 * #define API_ENDPOINT "/api"
 * #define API_PORT 80
 * // empty string for no auth or base64 encoded auth header value
 * #define API_AUTH_VALUE ""
 * 
 * // set to true to perform DNS lookup from API_HOST
 * // false to use fixed IP address
 * #define PERFORM_DNS_LOOKUP false
 * 
 * // components of fixed IP address
 * // only required if PERFORM_DNS_LOOKUP is false
 * #define API_IP1 127
 * #define API_IP2 0
 * #define API_IP3 0
 * #define API_IP4 1
 ****/

// initialise OneWire library
OneWire oneWire(ONE_WIRE_PIN);

// initialise temperature library
DallasTemperature sensors(&oneWire);

// IRQ must be an interrupt pin, VBAT and CS can be any pins
#define CC3000_IRQ  3
#define CC3000_VBAT 5
#define CC3000_CS   10
// hardware SPI is used for the remaning pins
// on an UNO, SCK = 13, MISO = 12, and MOSI = 11
// clock speed can be changed
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CC3000_CS, CC3000_IRQ, CC3000_VBAT, SPI_CLOCK_DIVIDER);

// amount of time in milliseconds to wait before closing connection with no data being receieved
#define IDLE_TIMEOUT_MS 3000

byte deviceCount;
byte deviceAddress[8];
byte deviceIndex;

uint32_t ip;

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
  String message = readSensors();

  Adafruit_CC3000_Client client = connect();

  if (client.connected()) {
    Serial.println(F("Connected to server"));
    
    send(client, message);
  
    client.close();
  
    // NB: connection must be cleaned up for the CC3000 can freak out on next connection
    Serial.println(F("Disconnecting..."));
    cc3000.disconnect();
  }
  else {
    Serial.println(F("Connection failed"));
  }
  
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
   * 
   * Example:
   *   { 0x28, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC }
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

String formatAddress(byte address[8]) {
  /**
   * Return the address of a device as a hexadecimal string.
   * 
   * Example:
   *   0x28ff123456789abc
   */
   
   String s = "0x";
   for (uint8_t i = 0; i < 8; i++) {
    // zero-pad the address if necessary
    if (address[i] < 16) {
      s += "0";
    }
    s += String(address[i], HEX);
   }
   return s;
}

String readSensors() {
  /**
   * Get temperature reading from each sensor.
   * Returns the readings as a JSON string (message to be sent to API).
   * 
   * Example message:
   *   { "readings": { "0x28ff123456789abc":14.44, "0x28ff123456789abd":14.38 } }
   * 
   * The address of each device is a unique 64-bit code stored in ROM (reference: DS18B20 datasheet).
   * The least significant 8-bits contains the 1-Wire family code (28h for the DS18B20).
   * The next 48 bits contain a unique serial number.
   * The most significant 8-bits contain a CRC calculated from the first 56 bits of the ROM code.
   */
   
  // call sensors.requestTemperatures() to issue a global temperature request to all devices in the bus
  Serial.println(F("Requesting temperatures..."));
  sensors.requestTemperatures();

  String message = "{ \"readings\": { ";
  for (deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
    if (sensors.getAddress(deviceAddress, deviceIndex)) {
      float temperature = sensors.getTempC(deviceAddress);
      String address = formatAddress(deviceAddress);
      Serial.print(F("Device "));
      Serial.print(deviceIndex);
      Serial.print(F("; address: "));
      printAddress(deviceAddress);
      Serial.print(F("; temperature: "));
      Serial.println(temperature);
      message = message + "\"" + address + "\":" + String(temperature);
      // comma-seperate readings (unless this is the last reading)
      if (deviceIndex < deviceCount - 1) {
        message += ", ";
      }
    } else {
      Serial.println(F("Unable to read from device"));
    }
  }
  message += " } }";
  Serial.println(F("Sensor data message:"));
  Serial.println(message);
  return message;
}

Adafruit_CC3000_Client connect() {
  /**
   * Connection to WiFi network and then to remote server.
   */
   
  // initialise module
  Serial.println(F("Initialising network connection..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Unable to initialise module"));
    while(1);
  }
  
  Serial.print(F("Attempting to connect to: ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Network connection failed"));
    while(1);
  }
   
  Serial.println(F("Connected to network"));
  
  // block until DHCP has been completed
  Serial.println(F("Starting DHCP..."));
  while (!cc3000.checkDHCP())
  {
    // TODO: timeout for DHCP lookup?
    delay(100);
  }
  Serial.println(F("DHCP complete"));

  if (PERFORM_DNS_LOOKUP) {
    Serial.println(F("Performing DNS lookup to get server IP address..."));
    ip = 0;
    while (ip == 0) {
      Serial.print(F("Attempting to resolve: "));
      Serial.println(API_HOST);
      int16_t dnsResult = cc3000.getHostByName(API_HOST, &ip);
      if (dnsResult > 0) {
        // testing indicates that a result of 1 indicates success
        Serial.print(F("IP address resolved; result code: "));
        Serial.println(dnsResult);
      } else {
        Serial.print(F("Unable to resolve; result code: "));
        Serial.println(dnsResult);
        delay(1000);
      }
    }
  } else {
    Serial.println(F("Not performing DNS lookup; using fixed server IP address"));
    ip = cc3000.IP2U32(API_IP1, API_IP2, API_IP3, API_IP4);
  }
    
  Serial.print(F("Connecting to IP: "));
  cc3000.printIPdotsRev(ip);
  Serial.println();
  
  // Use HTTP/1.1 to keep server from disconnecting before all data has been read
  return cc3000.connectTCP(ip, API_PORT);
}

void send(Adafruit_CC3000_Client client, String message) {
  /**
   * Send message to API as a POST request.
   */
   
  int contentLength = message.length();
  Serial.println(F("Sending request..."));
  Serial.print(F("Content-Length: "));
  Serial.println(contentLength, DEC);
  client.fastrprint(F("POST "));
  client.fastrprint(API_ENDPOINT);
  client.fastrprintln(F(" HTTP/1.1"));
  client.fastrprint(F("Host: "));
  client.fastrprintln(API_HOST);
  if (API_AUTH_VALUE != "") {
    Serial.println("Using HTTP Basic authentication");
    client.fastrprint(F("Authorization: Basic "));
    client.fastrprintln(API_AUTH_VALUE);
  } else {
    Serial.println("Not using authentication");
  }
  client.fastrprintln(F("User-Agent: Arduino/1.0"));
  client.fastrprintln(F("Content-Type: application/json"));
  client.fastrprintln(F("Connection: close"));
  client.fastrprint(F("Content-Length: "));
  client.println(contentLength, DEC);
  client.fastrprint(F("\r\n"));
  client.println(message);
  client.println();

  Serial.println(F("-------------------------------------"));
  
  // read data until either the connection is closed, or the idle timeout is reached
  unsigned long lastRead = millis();
  while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  
  Serial.println();
  Serial.println(F("-------------------------------------"));
}

