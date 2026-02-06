/*
 * config.h
 *
 *  Created on: Jan 6, 2026
 *      Author: huba
 */

#ifndef DZTHERMOSTAT_CONFIG_H_
#define DZTHERMOSTAT_CONFIG_H_

#define DEBUG
#define HAS_DISPLAY
#define USE_SSL_
#define ENYEM

// temperature sensors
#define freqReadTemperature 8 // 8 seconds between two temperature reading operation
#define temperatureSensorsCount 1

// OneWire
#define ONE_WIRE_BUS 10 // thermometer, OneWire communication, esp8266

// command buttons
#ifdef ENYEM
#define PIN_UP 16 // D0
#define PIN_DOWN 5 // D1
#else
#define PIN_UP 14 // D5
#define PIN_DOWN 12 // D6
#endif

#endif /* DZTHERMOSTAT_CONFIG_H_ */
