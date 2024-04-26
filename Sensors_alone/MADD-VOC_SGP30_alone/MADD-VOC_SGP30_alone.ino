#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class

void setup() {
  Serial.begin(115200);
  Wire.begin();
  //Initialize sensor
  if (mySensor.begin() == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }
  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
}

void loop() {
  //First fifteen readings will be
  //TVOC: 0 ppb
  delay(1000); //Wait 1 second
  mySensor.measureAirQuality();
  Serial.print("TVOC: ");
  Serial.print(mySensor.TVOC);
  Serial.print("ppb\t ");
  Serial.print(mySensor.TVOC * 0.0045);
  Serial.print("mg/m3\t ");
  Serial.print(mySensor.TVOC * 4.5);
  Serial.println("µg/m3");
}
