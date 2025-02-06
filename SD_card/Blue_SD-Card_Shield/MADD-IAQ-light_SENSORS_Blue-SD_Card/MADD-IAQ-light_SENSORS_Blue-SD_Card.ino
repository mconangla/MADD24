/****************************************
 *             Debug Flags
 ****************************************/
bool Monitor_PM25   = true;  // PM2.5 sensor prints
bool Monitor_SCD41  = true;  // SCD41 sensor prints
bool Monitor_BH1750 = true;  // BH1750 sensor prints
bool Monitor_SDCard = true;  // SD card prints
bool Monitor_log    = true;  // General/logging prints

/****************************************
 *           Libraries & Objects
 ****************************************/
#include <Arduino.h>
#include <Wire.h>

// SD Libraries
#include <SPI.h>
#include <SD.h>

// PM2.5 Libraries
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

// SCD41 Libraries
#include "SparkFun_SCD4x_Arduino_Library.h"

// BH1750 Library
#include <BH1750.h>

/****************************************
 *           Global Variables
 ****************************************/
const int chipSelect = D8;   // For the Wemos D1 mini SD shield, CS is typically D8
const long interval  = 20000; // logging interval, change it if you need a different loggin value (in miliseconds)

// ---------------------- Global Objects ---------------------
SoftwareSerial pmSerial(D4, D3); // PM sensor on D4 (RX) and D3 (TX)
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
SCD4x mySensor;
BH1750 lightMeter;
File myFile;

unsigned long previousMillis = 0;
String ValuesCache;
int lineNumber = 1; // Resets to number 1 on every power-up/reset


/****************************************
 *                Setup
 ****************************************/
void setup() {
  Serial.begin(115200);
  delay(3000);

  // ------------------ PM2.5 Setup ------------------
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {
    if (Monitor_PM25) {
      Serial.println("Could not find PM2.5 sensor!");
    }
    // Stop here if PM2.5 sensor not found
    while (1) { delay(10); }
  }
  if (Monitor_PM25) {
    Serial.println("PM2.5 sensor initialization successful.");
  }

  // ------------------ SCD41 Setup ------------------
  Wire.begin(); 
  if (!mySensor.begin()) {
    if (Monitor_SCD41) {
      Serial.println(F("SCD41 sensor not detected."));
    }
    // Stop here if SCD41 sensor not found
    while (1) { delay(10); }
  }
  if (Monitor_SCD41) {
    Serial.println("SCD41 initialization successful.");
  }

  // ------------------ BH1750 Setup ------------------
  if (Monitor_BH1750) {
    Serial.println("Initializing BH1750...");
  }
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    if (Monitor_BH1750) {
      Serial.println("BH1750 initialization successful.");
    }
  } else {
    if (Monitor_BH1750) {
      Serial.println("Error initializing BH1750!");
    }
    // Stop here if BH1750 sensor not found
    while (1) { delay(10); }
  }

  // ------------------ SD Card Setup ----------------
  if (Monitor_SDCard) {
    Serial.print("Initializing SD card...");
  }
  if (!SD.begin(chipSelect)) {
    if (Monitor_SDCard) {
      Serial.println("SD initialization failed!");
    }
    while (1) { delay(10); }
  }
  if (Monitor_SDCard) {
    Serial.println("SD initialization done.");
  }

  // If IAQ-light_data.csv does not exist, create and add a header row
  if (!SD.exists("IAQ-light_data.csv")) {
    myFile = SD.open("IAQ-light_data.csv", FILE_WRITE);
    if (myFile) {
      // Added "Light" column to the header
      myFile.println("Line,PM1.0,PM2.5,PM10,CO2,Temperature,Humidity,Light");
      myFile.close();
      if (Monitor_SDCard) {
        Serial.println("Created IAQ-light_data.csv and wrote header.");
      }
    }
  }
}

// --------------------- Read PM2.5 --------------------------
void readPM25() {
  PM25_AQI_Data data;

  // Wait until sensor gives valid data
  while (!aqi.read(&data)) {
    if (Monitor_PM25) {
      Serial.println("Waiting for PM2.5 sensor data...");
    }
    delay(500);
  }

  // Print PM2.5 data if enabled
  if (Monitor_PM25) {
    Serial.print(F("PM 1.0: "));
    Serial.print(data.pm10_standard);
    Serial.print(F("\tPM 2.5: "));
    Serial.print(data.pm25_standard);
    Serial.print(F("\tPM 10: "));
    Serial.println(data.pm100_standard);
  }

  // Prepare CSV portion
  ValuesCache += String(data.pm10_standard)  + ",";
  ValuesCache += String(data.pm25_standard)  + ",";
  ValuesCache += String(data.pm100_standard) + ",";
}

// --------------------- Read SCD41 --------------------------
void readSCD41() {
  // Wait for valid data from SCD41
  while (mySensor.readMeasurement() == 0) {
    if (Monitor_SCD41) {
      Serial.println("Waiting for SCD41 sensor data...");
    }
    delay(500);
  }

  // Print SCD41 data if enabled
  if (Monitor_SCD41) {
    Serial.print(F("CO2(ppm):"));
    Serial.print(mySensor.getCO2());
    Serial.print(F("\tTemperature(C):"));
    Serial.print(mySensor.getTemperature(), 1);
    Serial.print(F("\tHumidity(%RH):"));
    Serial.println(mySensor.getHumidity(), 1);
  }

  // Append SCD41 data
  ValuesCache += String(mySensor.getCO2())            + ",";
  ValuesCache += String(mySensor.getTemperature(), 1) + ",";
  ValuesCache += String(mySensor.getHumidity(), 1)    + ",";
}

// --------------------- Read BH1750 -------------------------
void readBH1750() {
  float lux = lightMeter.readLightLevel();

  // Check if reading is valid
  if (lux < 0) {
    if (Monitor_BH1750) {
      Serial.println("Error reading BH1750 light level!");
    }
    // Append "N/A" or similar if invalid
    ValuesCache += "N/A";
  } else {
    if (Monitor_BH1750) {
      Serial.print("Light Level: ");
      Serial.print(lux);
      Serial.println(" lux");
    }
    // Append BH1750 data
    ValuesCache += String(lux, 2); // 2 decimal places
  }
}

// --------------------- Write to SD -------------------------
void record_SD() {
  // Open in append mode
  myFile = SD.open("IAQ-light_data.csv", FILE_WRITE);
  if (myFile) {
    if (Monitor_SDCard) {
      Serial.println("Writing to IAQ-light_data.csv...");
    }
    // Prepend line number to the CSV row
    myFile.println(String(lineNumber) + "," + ValuesCache);
    myFile.close();

    if (Monitor_SDCard) {
      Serial.println("Data recorded to IAQ-light_data.csv");
    }
    // Increment for the next reading
    lineNumber++;
  } else {
    if (Monitor_SDCard) {
      Serial.println("Error opening IAQ-light_data.csv");
    }
  }
}

/****************************************
 *                Loop
 ****************************************/
void loop() {
  unsigned long currentMillis = millis();

  // Log data every 'interval' milliseconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Clear out old data from ValuesCache
    ValuesCache = "";

    // Read from sensors
    readPM25();
    readSCD41();
    readBH1750();

    // Write to SD card
    record_SD();

    // General debug print
    if (Monitor_log) {
      Serial.println("Valuecache: " + ValuesCache);
      Serial.println("--------------------------------");
    }
  }
}
