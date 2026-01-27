/*
 * config.h
 *
 *  Created on: Jan 6, 2026
 *      Author: huba
 */

#ifndef DZTERMOSTAT_CONFIG_H_
#define DZTERMOSTAT_CONFIG_H_

// #include "ds18b20_utils.h"

#define DEBUG_
#define HAS_DISPLAY
#define USE_SSL

// temperature sensors
#define freqReadTemperature 15 // 15 seconds between two temperature reading operation
// extern TTemperatureSensor temperatureSensors[];
#define temperatureSensorsCount 1

// OneWire
#define ONE_WIRE_BUS 10 // thermometer, OneWire communication, esp8266

// command buttons
#define PIN_UP 14 // D5
#define PIN_DOWN 12 // D6

#endif /* DZTERMOSTAT_CONFIG_H_ */
