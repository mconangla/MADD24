# Introduction
This repository contains a series of codes that are part of the Data Collection subject of the Master in Data Design (MADD) at Elisava. The codes aim to obtain data from different sensors to monitor air quality.
The following folders and files include, on one hand, codes that can be used separately to test each of the components, and on the other hand, files that combine different codes to use the components together.


# FOLDER > Sensors_alone
This folder contains all the codes to test the sensors separetely to check their performance. 
## MADD-IAQ_PMSA003_alone.ino
This code measures the concentration of airborne particulate matter, including PM1.0, PM2.5, and PM10 particles
## MADD_IAQ_SCD41_alone.ino
This code is used to measure carbon dioxide (CO2) concentration in the air, as well as temperature and humidity. 
## MADD-VOC_SGP30.ino
This code is used to measure TVOC (Total Volatile Organic Compounds) concentration in the air, measured in ppb (parts per billion)


# FOLDER > CoolTerm
This folder contains codes to check sensor combinations with CoolTerm, a program that allows to save the data directly through the serial port into a .txt file
## MADD-IAQ_SENSORS_CoolTerm.ino
Reads data from PM25 and SCD41 sensors. The data is prepared to be read by CoolTerm and exported into a .txt that can be imported into Excel with the columns split by values
## MADD-VOC_SENSORS_CoolTerm.ino
Same as MADD-IAQ_SENSORS_CoolTerm.ino but integrates SGP30 sensor's data 


# FOLDER > ThingSpeak
## MADD-IAQ_SENSORS_ThingSpeak.ino
Reads data from PM25 and SCD41 sensors at specified intervals using WiFi. The gathered data is then uploaded to a personalized channel in ThingSpeak.
## MADD-VOC_SENSORS_ThingSpeak.ino
Same as MADD-IAQ_SENSORS_ThingSpeak.ino but integrates SGP30 sensor's data

# FOLDER > Blynk
## MADD-IAQ_SENSORS_Blynk.ino
Reads data from PM25 and SCD41 sensors at specified intervals using WiFi. The gathered data is then uploaded to a personalized dashboard in Blynk.


# FOLDER > SD_card
## MADD-IAQ_SENSORS_SD_card.ino
Reads data from PM25 and SCD41 sensors at specified intervals, as well as the current date and time from the clock module. The gathered data is then stored in a data.csv file.


## MADD_IAQ_SD_card.ino
This code is responsible for saving and reading data to and from an SD card.

## MADD_IAQ_CLOCK_setup.ino
This code is used to initialize the clock module, set its parameters such as time format, time zone, and any other necessary settings.



# Conclusion
These programs collaborate to form a comprehensive data collection system for monitoring air quality and other environmental factors. They all can be changed or modified depending on personal needs and acording to the desired outcome. 
