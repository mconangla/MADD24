////// Program Control //////
bool Monitor_PM25 = true; //set to to true or false to enable or disable PM values through serial
bool Monitor_SCD41 = true; //set to to true or false to enable or disable CO2+TEMP+HUM values through serial
bool Monitor_SGP30 = true; //set to to true or false to enable or disable SGP30 CO2 and TVOC values through serial
bool Monitor_log = false; //set to to true or false to enable or disable WiFi+ThingSpeak details through serial

////// Time controller //////
unsigned long previousMillis = 0;
unsigned long interval = 10000;

///// PM25 Libraries ///// - Adafruit PM25 AQI
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
SoftwareSerial pmSerial(D4, D3);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data data;

///// SCD41 Libraries ///// - Sparkfun SCD4X
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySCD41;

///// SGP30 Libraries ///// 
#include "SparkFun_SGP30_Arduino_Library.h"
SGP30 mySGP30;

//// Log messages configuration /////
void logMessage(const String& message, bool monitor) {
  if (monitor) {
    Serial.println(message);
  }
}

void setup() {
  Serial.begin(115200);
  // Wait one second for sensor to boot up!
  delay(1000);

  setupSCD41();
  setupPM25();
  setupSGP30();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readSCD41();
    readPM25();
    readSGP30();
  }
}

////// SCD41 Setup //////
void setupSCD41() {
  Wire.begin();
  if (mySCD41.begin() == false) {
    logMessage("SCD41 Sensor not detected.", Monitor_log);
    while (1)
      ;
  }
}

////// PM25 Setup //////
void setupPM25() {
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // connect to the sensor over software serial
    logMessage("Could not find PM 2.5 sensor!", Monitor_log);
    while (1) delay(10);
  }
}

////// SGP30 Setup //////
void setupSGP30() {
  Wire.begin();
  if (mySGP30.begin() == false) {
    logMessage("No SGP30 Detected. Check connections.", Monitor_log);
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySGP30.initAirQuality();
}

////// SCD41 Reading function //////
void readSCD41() {
  while (mySCD41.readMeasurement() == 0) {
    if (Monitor_log) {
      logMessage("Waiting SCD41 sensor data...", Monitor_SCD41);
    }
    delay(500);
  }
  if (Monitor_SCD41) {
    Serial.print(mySCD41.getTemperature(), 1);
    Serial.print(F("\t"));
    Serial.print(mySCD41.getHumidity(), 1);
    Serial.print(F("\t"));
    Serial.print(mySCD41.getCO2());
  }
}

////// PM25 Reading function //////
void readPM25() {
  ////// PM25 Reading //////
  while (!aqi.read(&data)) {
    if (Monitor_log) {
      Serial.println("Waiting PM25 sensor data...");
    }
    delay(500);
  }
  if (Monitor_PM25) {
    Serial.print(F("\t"));
    Serial.print(data.pm10_standard);
    Serial.print(F("\t "));
    Serial.print(data.pm25_standard);
    Serial.print(F("\t "));
    Serial.print(data.pm100_standard);
  }
}

////// SGP30 Reading function //////
void readSGP30() {
  mySGP30.measureAirQuality();
  if (Monitor_SGP30) {
    Serial.print(F("\t "));
    Serial.print(mySGP30.TVOC);
    Serial.println();

  }
}
