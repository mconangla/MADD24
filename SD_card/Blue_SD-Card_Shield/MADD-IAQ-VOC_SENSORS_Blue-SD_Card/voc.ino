#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// PM2.5 Libraries
#include <SoftwareSerial.h>
#include "Adafruit_PM25AQI.h"

// SCD41 Libraries
#include "SparkFun_SCD4x_Arduino_Library.h"

// SGP30 Libraries
#include "SparkFun_SGP30_Arduino_Library.h" // http://librarymanager/All#SparkFun_SGP30

// ---------------------- SD Configuration ----------------------
const int chipSelect = D8; // For the Wemos D1 mini SD shield, CS is typically D8
const long interval  = 20000; // logging interval in milliseconds

// ---------------------- Global Objects ------------------------
SoftwareSerial pmSerial(D4, D3); // PM sensor on D4 (RX) and D3 (TX)
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

SCD4x mySensor;       // For SCD41
SGP30 mySGP30;        // For SGP30

File myFile;

// ---------------------- Variables & Flags ---------------------
unsigned long previousMillis = 0;
String ValuesCache;
int lineNumber = 1; // Resets to number 1 on every power-up/reset

// Debug flags
bool Monitor_PM25   = true;  // PM2.5 sensor prints
bool Monitor_SCD41  = true;  // SCD41 sensor prints
bool Monitor_SGP30  = true;  // SGP30 sensor prints
bool Monitor_SDCard = true;  // SD card prints
bool Monitor_log    = true;  // General/logging prints

// ------------------------- Setup -----------------------------
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

  // ------------------ SGP30 Setup ------------------
  if (!mySGP30.begin()) {
    if (Monitor_SGP30) {
      Serial.println("No SGP30 Detected. Check connections.");
    }
    // Stop here if SGP30 sensor not found
    while (1) { delay(10); }
  }
  if (Monitor_SGP30) {
    Serial.println("SGP30 initialization successful.");
  }
  // Initialize air quality readings
  mySGP30.initAirQuality();

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

  // If data.csv does not exist, create and add a header row
  if (!SD.exists("data.csv")) {
    myFile = SD.open("data.csv", FILE_WRITE);
    if (myFile) {
      // Add new columns for SGP30 data (TVOC in ppb, mg/m3, and µg/m3)
      myFile.println("Line,PM1.0,PM2.5,PM10,CO2,Temperature,Humidity,TVOC_ppb,TVOC_mg_m3,TVOC_ug_m3");
      myFile.close();
      if (Monitor_SDCard) {
        Serial.println("Created data.csv and wrote header.");
      }
    }
  }
}

// --------------------- Read PM2.5 ---------------------------
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

// --------------------- Read SCD41 ---------------------------
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

// --------------------- Read SGP30 ---------------------------
void readSGP30() {
  // For accuracy, measureAirQuality() is typically called every 1s,
  // but here we'll do it once per interval for demonstration.
  mySGP30.measureAirQuality();

  if (Monitor_SGP30) {
    Serial.print("TVOC: ");
    Serial.print(mySGP30.TVOC);
    Serial.print(" ppb\t ");
    Serial.print(mySGP30.TVOC * 0.0045);
    Serial.print(" mg/m3\t ");
    Serial.print(mySGP30.TVOC * 4.5);
    Serial.println(" µg/m3");
  }

  // Append SGP30 data
  // (Column 1) TVOC in ppb
  // (Column 2) TVOC in mg/m3
  // (Column 3) TVOC in µg/m3
  ValuesCache += String(mySGP30.TVOC)                + ",";
  ValuesCache += String(mySGP30.TVOC * 0.0045, 4)    + ",";
  ValuesCache += String(mySGP30.TVOC * 4.5, 1);
}

// --------------------- Write to SD --------------------------
void record_SD() {
  // Open in append mode
  myFile = SD.open("data.csv", FILE_WRITE);
  if (myFile) {
    if (Monitor_SDCard) {
      Serial.println("Writing to data.csv...");
    }
    // Prepend line number to the CSV row
    myFile.println(String(lineNumber) + "," + ValuesCache);
    myFile.close();

    if (Monitor_SDCard) {
      Serial.println("Data recorded to data.csv");
    }
    // Increment for the next reading
    lineNumber++;
  } else {
    if (Monitor_SDCard) {
      Serial.println("Error opening data.csv");
    }
  }
}

// ------------------------- Loop -----------------------------
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
    readSGP30();

    // Write to SD card
    record_SD();

    // General debug print
    if (Monitor_log) {
      Serial.println("Valuecache: " + ValuesCache);
      Serial.println("--------------------------------");
    }
  }
}
