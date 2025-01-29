////// Program Control //////
bool Monitor_PM25 = true; //set to to true or false to enable or disable PM values through serial
bool Monitor_SCD41 = true; //set to to true or false to enable or disable CO2+TEMP+HUM values through serial
bool Monitor_log = true; //set to to true or false to enable or disable WiFi+ThingSpeak details through serial

////// Time controller //////
unsigned long previousMillis = 0;
unsigned long interval = 20000;

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

//// Log messages configuration /////
void logMessage(const String& message, bool monitor) {
  if (monitor) {
    Serial.println(message);
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000); // Wait one second for sensor to boot up!

  setupPM25();
  setupSCD41();
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
  if (!aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial
    logMessage("Could not find PM 2.5 sensor!", Monitor_log);
    while (1) delay(10);
  } else {
    logMessage("PM2.5 sensor initialization successful.", Monitor_log);
  }
}

////// SCD41 Setup //////
void setupSCD41() {
  // Initialize I2C on D2 (SDA) and D1 (SCL)
  Wire.begin(D2, D1);

  if (!mySensor.begin()) {
    logMessage("SCD41 Sensor not detected.", Monitor_log);
    while (1);
  } else {
    logMessage("SCD41 sensor initialization successful.", Monitor_log);
  }
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
  while (mySensor.readMeasurement() == 0) {
    if (Monitor_log) {
      logMessage("Waiting SCD41 sensor data...", Monitor_SCD41);
    }
    delay(500);
  }
  if (Monitor_SCD41) {
    Serial.print(F("Temperature:"));
    Serial.print(mySensor.getTemperature(), 1);
    Serial.print("ºC\t");
    Serial.print(F("Humidity:"));
    Serial.print(mySensor.getHumidity(), 1);
    Serial.print("%RH\t");
    Serial.print(F("CO2:"));
    Serial.print(mySensor.getCO2());
    Serial.print("ppm");
    Serial.println();
  }
}

//// Thingspeak Writting function /////
void writeThingSpeak() {
  // set the fields with the values
  ThingSpeak.setField(1, mySensor.getTemperature());
  ThingSpeak.setField(2, mySensor.getHumidity());
  ThingSpeak.setField(3, mySensor.getCO2());
  ThingSpeak.setField(4, data.pm10_standard);
  ThingSpeak.setField(5, data.pm25_standard);
  ThingSpeak.setField(6, data.pm100_standard);
  
  // Write to ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  // Log the result if Monitor_log is true
  if (Monitor_log) {
    // Print serial messages if Monitor_log is true
    if (x == 200) {
      logMessage("Channel update successful", Monitor_log);
    } else {
      logMessage("Problem updating channel. HTTP error code: " + String(x), Monitor_log);
    }
  }
}
