String ValuesCache, ValuesRecord;
////// Program Control //////
bool Monitor_Time = true;
bool Monitor_PM25 = true;
bool Monitor_SCD41 = true;
bool Monitor_SDCard = true;
bool Monitor_log = false;

////// Time controller //////
unsigned long previousMillis = 0;
const long interval = 10000;

////// Clock Libraries //////
#include "RTClib.h"
RTC_DS1307 rtc;

////// SCD41 Libraries ////// - Sparkfun SCD4X
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySensor;

////// PM25 Libraries ////// - Adafruit PM25 AQI
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
SoftwareSerial pmSerial(D4, D3);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

////// SDCard Libraries //////
#include <SPI.h>
#include <SD.h>
const int chipSelect = D8;
File myFile;

void setup() {
  Serial.begin(115200);
  // Wait one second for sensor to boot up!
  delay(1000);

  ////// Clock Setup //////
#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  ////// PM25 Setup //////
  pmSerial.begin(9600);
  if (!aqi.begin_UART(&pmSerial)) {  // connect to the sensor over software serial
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


void readTime() {
  ////// TIME Reading //////
  DateTime time = rtc.now();

  if (Monitor_Time) {
    Serial.println(F("---------------------------------------"));
    Serial.println(String("DateTime:\t") + time.timestamp(DateTime::TIMESTAMP_FULL));
  }

  //Save the time value to variable
  ValuesCache = String("\t") + time.timestamp(DateTime::TIMESTAMP_FULL);
}

void readPM25() {
  ////// PM25 Reading //////
  PM25_AQI_Data data;

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

  //Save the time value to variable
  ValuesCache = ValuesCache + "," + data.pm10_standard + "," + data.pm25_standard + "," + data.pm100_standard;
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

    Serial.print(F("CO2(ppm):"));
    Serial.print(mySensor.getCO2());

    Serial.print(F("\tTemperature(C):"));
    Serial.print(mySensor.getTemperature(), 1);

    Serial.print(F("\tHumidity(%RH):"));
    Serial.print(mySensor.getHumidity(), 1);

    Serial.println();
  }
  //Save the time value to variable
  ValuesCache = ValuesCache + "," + mySensor.getCO2() + "," + mySensor.getTemperature() + "," + mySensor.getHumidity();
}

void record_SD() {

  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    return;
  }
  //Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("data.csv", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {


    if (Monitor_PM25) {
      Serial.println("Writing to data.csv...");
    }

    myFile.println(ValuesCache);
    // close the file:
    myFile.close();

  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening data.csv");
  }
}

void loop() {
  ////// Time controller //////
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ValuesCache = "";
    readTime();    // 1) Print date/time first
    readPM25();    // 2) Print PM2.5 readings
    readSCD41();   // 3) Print CO2/Temp/Humidity
    record_SD();   // 4) Write to SD card
    Serial.println(String("Valuecache: ") + ValuesCache); //5) print final line
  }
}
