/****************************************
 *             Debug Flags
 ****************************************/
bool Monitor_Time   = true;
bool Monitor_PM25   = true;
bool Monitor_SCD41  = true;
bool Monitor_SDCard = true;
bool Monitor_log    = true;

/****************************************
 *           Libraries & Objects
 ****************************************/
#include <Arduino.h>
#include "RTClib.h"                // Real Time Clock
#include <Wire.h>                  // I2C
#include "SparkFun_SCD4x_Arduino_Library.h"  // SCD41
#include "Adafruit_PM25AQI.h"      // PM2.5
#include <SoftwareSerial.h>        // For PM2.5 sensor
#include <SPI.h>                   // SD
#include <SD.h>                    // SD

/****************************************
 *           Global Variables
 ****************************************/
String ValuesCache;           // Where readings for one line are built
unsigned long previousMillis = 0;
const long interval = 20000;  // 20 seconds

/****************************************
 *           Pin Definitions
 ****************************************/
// For the Wemos D1 mini SD shield, CS is typically D8
const int chipSelect = D8;

// PM2.5 sensor on pins D4 (RX) and D3 (TX)
SoftwareSerial pmSerial(D4, D3);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

SCD4x mySensor;      // SCD41 object

// Real Time Clock
RTC_DS1307 rtc;

// SD card file handle
File myFile;

/****************************************
 *           Setup
 ****************************************/
void setup() {
  Serial.begin(115200);
  // Wait three seconds for sensors to boot
  delay(3000);

  // ---------------------- RTC Setup ----------------------
#ifndef ESP8266
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) {
      delay(10);
    }
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss:
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // --------------------- PM2.5 Setup ----------------------
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) {
      delay(10);
    }
  }
  if (Monitor_PM25) {
    Serial.println("PM2.5 sensor initialization successful.");
  }

  // --------------------- SCD41 Setup ----------------------
  Wire.begin();
  if (!mySensor.begin()) {
    if (Monitor_SCD41) {
      Serial.println(F("SCD41 sensor not detected."));
    }
    // Stop here if SCD41 sensor not found
    while (1) {
      delay(10);
    }
  }
  if (Monitor_SCD41) {
    Serial.println("SCD41 initialization successful.");
  }

  // --------------------- SD Card Setup --------------------
  if (Monitor_SDCard) {
    Serial.print("Initializing SD card... ");
  }
  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    while (1) {
      delay(10);
    }
  } else {
    Serial.println("SD initialization done.");
  }

  // If data.csv does not exist, create it and add a header row
  if (!SD.exists("Time-IAQ_data.csv")) {
    myFile = SD.open("Time-IAQ_data.csv", FILE_WRITE);
    if (myFile) {
      // Customize your header as needed:
      myFile.println("DateTime,PM1.0,PM2.5,PM10,CO2,Temperature,Humidity");
      myFile.close();
      if (Monitor_SDCard) {
        Serial.println("Created Time-IAQ_data.csv and wrote header.");
      }
    } else {
      Serial.println("Error creating Time-IAQ_data.csv");
    }
  }
}

// ------------------ Time Setup ------------------
void readTime() {
  // Read RTC time
  DateTime time = rtc.now();

  if (Monitor_Time) {
    Serial.println(F("---------------------------------------"));
    Serial.println(String("DateTime:\t") + time.timestamp(DateTime::TIMESTAMP_FULL));
  }
  // Save time to the CSV line
  ValuesCache = time.timestamp(DateTime::TIMESTAMP_FULL);
}

  // ------------------ PM2.5 Setup ------------------
void readPM25() {
  PM25_AQI_Data data;

  // Wait until we get valid data
  while (!aqi.read(&data)) {
    if (Monitor_log) {
      Serial.println("Waiting PM25 sensor data...");
    }
    delay(500);
  }

  if (Monitor_PM25) {
    Serial.print(F("PM 1.0: "));
    Serial.print(data.pm10_standard);
    Serial.print(F("\t\tPM 2.5: "));
    Serial.print(data.pm25_standard);
    Serial.print(F("\t\tPM 10: "));
    Serial.println(data.pm100_standard);
  }

  // Append PM data to the CSV line
  ValuesCache += "," + String(data.pm10_standard);
  ValuesCache += "," + String(data.pm25_standard);
  ValuesCache += "," + String(data.pm100_standard);
}

  // ------------------ SCD41 Setup ------------------
void readSCD41() {
  // Wait until we get valid data
  while (mySensor.readMeasurement() == 0) {
    if (Monitor_log) {
      Serial.println("Waiting SCD41 sensor data...");
    }
    delay(500);
  }

  if (Monitor_SCD41) {
    Serial.print(F("CO2(ppm):"));
    Serial.print(mySensor.getCO2());
    Serial.print(F("\tTemperature(C):"));
    Serial.print(mySensor.getTemperature(), 1);
    Serial.print(F("\tHumidity(%RH):"));
    Serial.println(mySensor.getHumidity(), 1);
  }

  // Append SCD41 data to the CSV line
  ValuesCache += "," + String(mySensor.getCO2());
  ValuesCache += "," + String(mySensor.getTemperature(), 1);
  ValuesCache += "," + String(mySensor.getHumidity(), 1);
}

// ------------------ SD Card Setup ----------------
void record_SD() {
  // Open the file for appending
  myFile = SD.open("Time-IAQ_data.csv", FILE_WRITE);
  if (myFile) {
    if (Monitor_PM25) {
      Serial.println("Writing to Time-IAQ_data.csv...");
    }
    // Write the entire CSV line to the file
    myFile.println(ValuesCache);
    // Close the file
    myFile.close();
  } else {
    Serial.println("Error opening Time-IAQ_data.csv");
  }
}

/****************************************
 *                Loop
 ****************************************/
void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to log
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Clear old data
    ValuesCache = "";

    // 1) Read date/time
    readTime();
    // 2) Read PM2.5
    readPM25();
    // 3) Read SCD41 (CO2, temp, humidity)
    readSCD41();
    // 4) Write it all to SD
    record_SD();

    // Debug print final line
    if (Monitor_log) {
      Serial.println(String("Valuecache: ") + ValuesCache);
    }
  }
}
