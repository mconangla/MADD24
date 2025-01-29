#include <Wire.h>
#include <BH1750.h>

// Create a BH1750 object
BH1750 lightMeter;

void setup() {
  Serial.begin(115200); // Start serial communication
  Serial.println("BH1750 Test");

  Wire.begin(D2, D1); // Initialize I2C (SDA, SCL)

  if (lightMeter.begin()) {
    Serial.println("BH1750 initialized");
  } else {
    Serial.println("Error initializing BH1750");
  }
}

void loop() {
  // Read light intensity in lux
  float lux = lightMeter.readLightLevel();

  // Check if reading is valid
  if (lux < 0) {
    Serial.println("Error reading light level!");
  } else {
    Serial.print("Light Level: ");
    Serial.print(lux);
    Serial.println(" lux");
  }

  delay(2000); // Wait 2 seconds before next reading
}
