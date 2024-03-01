# Introduction
This repository contains a series of codes that are part of the Data Collection subject of the Master in Data Design (MADD) at Elisava. The codes aim to obtain data from different sensors to monitor air quality.

# Overview
The following files include, on one hand, codes that can be used separately to test each of the components, and on the other hand, files that combine different codes to use the components together.

# Codes. Components alone
## MADD_IAQ_CLOCK_setup.ino
This code is used to initialize the clock module, set its parameters such as time format, time zone, and any other necessary settings.

## MADD-IAQ_PMSA003_alone.ino
This code measures the concentration of airborne particulate matter, including PM1.0, PM2.5, and PM10 particles

## MADD_IAQ_SCD41_alone.ino
This code is used to measure carbon dioxide (CO2) concentration in the air, as well as temperature and humidity. 

## MADD_IAQ_SD_card.ino
This code is responsible for saving and reading data to and from an SD card.

# Codes. Components combination
## MADD-IAQ_SENSORS_ThingSpeak.ino
Reads data from PM25 and SCD41 sensors at specified intervals using WiFi. The gathered data is then uploaded to a personalized channel in ThingSpeak.

## MADD-IAQ_SENSORS_Blynk.ino
Reads data from PM25 and SCD41 sensors at specified intervals using WiFi. The gathered data is then uploaded to a personalized dashboard in Blynk.

## MADD-IAQ_SENSORS_SD_card.ino
Reads data from PM25 and SCD41 sensors at specified intervals, as well as the current date and time from the clock module. The gathered data is then stored in a data.csv file.

# Conclusion
These programs collaborate to form a comprehensive data collection system for monitoring air quality and other environmental factors. They all can be changed or modified depending on personal needs and acording to the desired outcome. 
