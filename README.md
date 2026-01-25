# AVR Controlled AirQualityMonitor
An AVR controlled air quality monitoring system. This is a system composed of a base unit and sensor modules. The sensor modules take in data using the following array of sensors:

DHT22: Temperature and Humidity
BMP280: Atmospheric Pressure/Elevation
MiCS-5524: VOC detection of CO, ethanol, H2, CH4 and NH3

The data is then processed by the ATMEGA328-P and can be displayed on an LCD at each sensor array. The data is then transmitted to the base unit, where it is logged, analyzed and converted to graphs, data-tables. The base station microprocesser than can transmit raw data, graphes etc via Wifi to an email or local website, etc for viewing from anywhere. 

This repository contains the following:
1) Microcontroller firmware for the sensor arrays
2) Microprocesser firmware for the base station
3) Schematics, PCB design files for the device generated through KiCad
4) HTML, Javascript, etc for locally hosted website
5) Associated files and information about the design of the device, its function and how it operates.

**DISCLAIMER**
This device is created as an FCC Part 15 device and is unlicensed and not FCC-certified. The schematics, files, and information contained in this repository are for personal/educational purposes only and provided "as-is." Using the information contained here to design a comparable device does not ensure or guarantee that said device will not cause radio interference, and caution/proper engineering principles should be followed to ensure your device is in compliance with part 15. The creator of this repository created the following files in the course of creating a device under the homemade exception listed in 47 CFR § 15.23. It is the responsibility of any builder of the device specified in these files to ensure that their device complies with all local/federal laws and regulations; the designer of these files does not take responsibility for or ensure the functioning of any device created from these files. 

