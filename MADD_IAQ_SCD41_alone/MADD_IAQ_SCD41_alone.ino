#include <Wire.h>

#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x
SCD4x mySensor;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("SCD41 Example"));
  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to get helpful debug messages on Serial

  //.begin will start periodic measurements for us (see the later examples for details on how to override this)
  if (mySensor.begin() == false)
  {
    Serial.println(F("SCD41 Sensor not detected"));
    while (1)
      ;
  }
  //The SCD4x has data ready every five seconds
}

void loop()
{
  if (mySensor.readMeasurement()) // readMeasurement will return true when fresh data is available
  {
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
  delay(500);
}
