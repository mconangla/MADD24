/****************************************
 *             Debug Flags
 ****************************************/
bool Monitor_Time   = true;  // RTC prints
bool Monitor_PM25   = true;  // PM2.5 sensor prints
bool Monitor_SCD41  = true;  // SCD41 sensor prints
bool Monitor_SDCard = true;  // SD card prints
bool Monitor_log    = true;  // General/logging prints
bool Monitor_SGP30  = true;  // SGP30 sensor prints

/****************************************
 *           Libraries & Objects
 ****************************************/
#include <Arduino.h>
#include "RTClib.h"                             // Real Time Clock
#include <Wire.h>                               // I2C
#include "SparkFun_SCD4x_Arduino_Library.h"     // SCD41
#include "Adafruit_PM25AQI.h"                   // PM2.5
#include <SoftwareSerial.h>                     // For PM2.5 sensor
#include <SPI.h>                                // SD
#include <SD.h>                                 // SD
#include "SparkFun_SGP30_Arduino_Library.h"     // SGP30

/****************************************
 *           Global Variables
 ****************************************/
String ValuesCache;           // Where readings for one line are built
unsigned long previousMillis = 0;
const long interval = 20000;  // 20 seconds (adjust if desired)

/****************************************
 *           Pin Definitions
 ****************************************/
// For the Wemos D1 mini SD shield, CS is typically D8
const int chipSelect = D8;

// PM2.5 sensor on pins D4 (RX) and D3 (TX)
SoftwareSerial pmSerial(D4, D3);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

// SCD41 object
SCD4x mySensor;      

// Real Time Clock
RTC_DS1307 rtc;

// SGP30 VOC sensor object
SGP30 mySGP30;

// SD card file handle
File myFile;

/****************************************
 *                Setup
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
  if (!SD.exists("Time-IAQ-VOC_data.csv")) {
    myFile = SD.open("Time-IAQ-VOC_data.csv", FILE_WRITE);
    if (myFile) {
      // CSV header row. 
      // Now including columns for TVOC in ppb, mg/m^3, and µg/m^3
      myFile.println("DateTime,PM1.0,PM2.5,PM10,CO2,Temperature,Humidity,TVOC_ppb,TVOC_mg_m3,TVOC_ug_m3");
      myFile.close();
      if (Monitor_SDCard) {
        Serial.println("Created Time-IAQ-VOC_data.csv and wrote header.");
      }
    } else {
      Serial.println("Error creating Time-IAQ-VOC_data.csv");
    }
  }
}

/****************************************
 *             Read Functions
 ****************************************/

// ------------------ Time ------------------
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

// ------------------ PM2.5 ------------------
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

// ------------------ SCD41 ------------------
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
  ValuesCache += "," + String(mySGP30.TVOC);
  ValuesCache += "," + String(mySGP30.TVOC * 0.0045, 4);
  ValuesCache += "," + String(mySGP30.TVOC * 4.5, 1);
}

// ------------------ SD Card ----------------
void record_SD() {
  // Open the file for appending
  myFile = SD.open("Time-IAQ-VOC_data.csv", FILE_WRITE);
  if (myFile) {
    if (Monitor_log) {
      Serial.println("Writing to Time-IAQ-VOC_data.csv...");
    }
    // Write the entire CSV line to the file
    myFile.println(ValuesCache);
    // Close the file
    myFile.close();
  } else {
    Serial.println("Error opening Time-IAQ-VOC_data.csv");
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
    // 4) Read VOC (SGP30)
    readSGP30();
    // 5) Write it all to SD
    record_SD();

    // Debug print final line
    if (Monitor_log) {
      Serial.println(String("Valuecache: ") + ValuesCache);
    }
  }
}
