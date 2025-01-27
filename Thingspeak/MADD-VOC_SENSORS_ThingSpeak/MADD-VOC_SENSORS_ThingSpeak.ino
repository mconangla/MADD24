////// Program Control //////
bool Monitor_PM25 = true; //set to to true or false to enable or disable PM values through serial
bool Monitor_SCD41 = true; //set to to true or false to enable or disable CO2+TEMP+HUM values through serial
bool Monitor_SGP30 = true; //set to to true or false to enable or disable SGP30 CO2 and TVOC values through serial
bool Monitor_log = true; //set to to true or false to enable or disable WiFi+ThingSpeak details through serial

////// Time controller //////
unsigned long previousMillis = 0;
unsigned long interval = 20000;

///// PM25 Libraries ///// - Adafruit PM25 AQI
#include "Adafruit_PM25AQI.h"
#include <SoftwareSerial.h>
SoftwareSerial pmSerial(D4, D3)
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data data;

///// SCD41 Libraries ///// - Sparkfun SCD4X
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySCD41;

///// SGP30 Libraries ///// 
#include "SparkFun_SGP30_Arduino_Library.h"
SGP30 mySGP30;

///// Thingspeak libraries /////
#include <ESP8266WiFi.h>
#include "ThingSpeak.h" 

///// WiFi configuration /////
//change these names  with your own Wifi setup!
char ssid[] = "YOUR_WIFI_NAME_HERE";   // your network SSID (name) 
char pass[] = "YOUR_WIFI_PASSWORD_HERE";   // your network password
int keyIndex = 0;   // your network key Index number (needed only for WEP)
WiFiClient  client;

///// ThingSpeak configuration /////
//change these numbers with your own Thingspeak setup!
unsigned long myChannelNumber = YOUR_CHANNEL_NUMBER_HERE;
const char * myWriteAPIKey = "YOUR_WRITE_API_KEY_HERE";

int number = 0;

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

  setupPM25();
  setupSCD41();
  setupSGP30();
  setupWiFi();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    connectThingspeak();
    updateSensorsAndSendData();
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

////// SCD41 Setup //////
void setupSCD41() {
  Wire.begin();
  if (mySCD41.begin() == false) {
    logMessage("SCD41 Sensor not detected.", Monitor_log);
    while (1);
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

///// Wifi Setup /////
void setupWiFi() {
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

//// Thingspeak Connection function /////
void connectThingspeak() {
  if(WiFi.status() != WL_CONNECTED) { // Connect or reconnect to WiFi
    logMessage("Attempting to connect to SSID: " + String(ssid), Monitor_log); 
    while(WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print("."); 
      delay(5000);     
    } 
    logMessage("\nConnected.", Monitor_log); // Unified monitoring flag
  }
}

//// Read+write Data function /////
void updateSensorsAndSendData() {
  readPM25();
  readSCD41();
  readSGP30();
  writeThingSpeak();
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
    Serial.print(F("PM1.0:"));
    Serial.print(data.pm10_standard);
    Serial.print("µg/m3\t");
    Serial.print(F("PM2.5:"));
    Serial.print(data.pm25_standard);
    Serial.print("µg/m3\t");
    Serial.print(F("PM10:"));
    Serial.print(data.pm100_standard);
    Serial.print("µg/m3");
    Serial.println();
  }
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
    Serial.print(F("Temperature:"));
    Serial.print(mySCD41.getTemperature(), 1);
    Serial.print("ºC\t");
    Serial.print(F("Humidity:"));
    Serial.print(mySCD41.getHumidity(), 1);
    Serial.print("%RH\t");
    Serial.print(F("CO2:"));
    Serial.print(mySCD41.getCO2());
    Serial.print("ppm");
    Serial.println();
  }
}

////// SGP30 Reading function //////
void readSGP30() {
  mySGP30.measureAirQuality();
  if (Monitor_SGP30) {
    //First fifteen readings will be
    //TVOC: 0 ppb
    Serial.print(F("TVOC: "));
    Serial.print(mySGP30.TVOC);
    Serial.println(" ppb");
    //Serial.print(mySensor.TVOC * 0.0045);
    //Serial.print("mg/m3\t ");
    //Serial.print(mySGP30.TVOC * 4.5);
    //Serial.println("µg/m3");
  }
}

//// Thingspeak Writting function /////
void writeThingSpeak() {
  // set the fields with the values
  ThingSpeak.setField(1, mySCD41.getTemperature());
  ThingSpeak.setField(2, mySCD41.getHumidity());
  ThingSpeak.setField(3, mySCD41.getCO2());
  ThingSpeak.setField(4, data.pm10_standard);
  ThingSpeak.setField(5, data.pm25_standard);
  ThingSpeak.setField(6, data.pm100_standard);
  ThingSpeak.setField(7, mySGP30.TVOC);
  
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (Monitor_log) {
    // Print serial messages if Monitor_log is true
    if (x == 200) {
      logMessage("Channel update successful", Monitor_log);
    } else {
      logMessage("Problem updating channel. HTTP error code: " + String(x), Monitor_log);
    }
  }
}
