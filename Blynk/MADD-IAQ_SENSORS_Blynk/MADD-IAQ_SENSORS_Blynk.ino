////// Program Control //////
bool Monitor_PM25 = true;
bool Monitor_SCD41 = true;
bool Monitor_log = false;

///// PM25 Libraries ///// - Adafruit PM25 AQI
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
SoftwareSerial pmSerial(D4, D3);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data data;

///// SCD41 Libraries ///// - Sparkfun SCD4X
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySensor;

///// Blynk libraries /////
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
#define BLYNK_TEMPLATE_NAME         "Device"
#define BLYNK_AUTH_TOKEN            "YourAuthToken"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  // Wait one second for sensor to boot up!
  delay(1000);

  ////// Blynk Setup //////
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(30000L, myTimer);  // Timer runs every 30 seconds

  ////// PM25 Setup //////
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // Connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  ////// SCD41 Setup //////
  Wire.begin();
  if (mySensor.begin() == false) {
    Serial.println(F("SCD41 Sensor not detected."));
    while (1)
      ;
  }
}

void loop() {
  Blynk.run();
  timer.run();  // Initiates BlynkTimer
}

void myTimer() {
  readSCD41();
  readPM25();
  writeBlynk();
}

void readSCD41() {
  ////// SCD41 Reading //////
  while (mySensor.readMeasurement() == 0) {
    if (Monitor_log) {
      Serial.println("Waiting SCD41 sensor data...");
    }
    delay(500);
  }
  if (Monitor_SCD41) {
    Serial.print(F("Temperature: "));
    Serial.print(mySensor.getTemperature(), 1);
    Serial.print(" °C\t");
    Serial.print(F("Humidity: "));
    Serial.print(mySensor.getHumidity(), 1);
    Serial.print(" %RH\t");
    Serial.print(F("CO2: "));
    Serial.print(mySensor.getCO2());
    Serial.print(" ppm");
    Serial.println();
  }
}

void readPM25() {
  ////// PM25 Reading //////
  while (!aqi.read(&data)) {
    if (Monitor_log) {
      Serial.println("Waiting PM25 sensor data...");
    }
    delay(500);
  }
  if (Monitor_PM25) {
    Serial.print(F("PM1.0: "));
    Serial.print(data.pm10_standard);
    Serial.print(" µg/m3\t");
    Serial.print(F("PM2.5: "));
    Serial.print(data.pm25_standard);
    Serial.print(" µg/m3\t");
    Serial.print(F("PM10: "));
    Serial.print(data.pm100_standard);
    Serial.print(" µg/m3");
    Serial.println();
  }
}

void writeBlynk() {
  //// Blynk Writing /////
  // Set the fields with the values
  Blynk.virtualWrite(V1, mySensor.getTemperature());
  Blynk.virtualWrite(V2, mySensor.getHumidity());
  Blynk.virtualWrite(V3, mySensor.getCO2());
  Blynk.virtualWrite(V4, data.pm10_standard);
  Blynk.virtualWrite(V5, data.pm25_standard);
  Blynk.virtualWrite(V6, data.pm100_standard);
  Serial.println("Sensors' data uploaded to Blynk");
}
