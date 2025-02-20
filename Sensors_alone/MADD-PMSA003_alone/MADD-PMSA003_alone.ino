/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */

#include "Adafruit_PM25AQI.h"

// If your PM2.5 is UART only, for UNO and others (without hardware serial) 
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
#include <SoftwareSerial.h>
const int sensorRxPin = D4;
const int sensorTxPin = D3;
SoftwareSerial pmSerial(sensorRxPin, sensorTxPin);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

void setup() {
  // Debugging serial for wemos-computer communication
  // Wait for serial monitor to open
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait three seconds for sensor to boot up!
  delay(3000);

  // If using serial, initialize it and set baudrate before starting!
  // Uncomment one of the following
  //Serial1.begin(9600);
  pmSerial.begin(9600);

  // There are 3 options for connectivity!
  //if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  //if (! aqi.begin_UART(&Serial1)) { // connect to the sensor over hardware serial
  if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial 
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
}

void loop() {
  PM25_AQI_Data data;
  
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(1000);  // try again in a bit!
    return;
  }

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

  delay(5000);
}
