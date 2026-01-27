#include "ds18b20_utils.h"
// #include <utils.h>
#include "config.h"

extern TTemperatureSensor temperatureSensors[]; // defined in config.cpp

/**
 * convert a sensor address to a string format
 */
String addressToStr(DeviceAddress addr) {
	String s = "";
	for (uint8_t i = 0; i < 8; i++) {
		if (addr[i] < 16) s += String(0, HEX);
		s += String(addr[i], HEX);
	}
	return s;
}

/**
 * Return a string about the found temperature sensors
 */
String enumTemperatureSensors(DallasTemperature &dtSensors) {
	DeviceAddress deviceAddress; // temperature sensor address
	int temperatureSensorsFound = dtSensors.getDeviceCount();
	if (temperatureSensorsFound <= 0) { return "false"; }

	String s = "found " + String(temperatureSensorsFound) + " temperature sensors\n";

	// loop through the temperature sensors and print their addresses
	for (uint8_t i = 0; i < temperatureSensorsFound; i++) {
		if (dtSensors.getAddress(deviceAddress, i)) {
			s += "device " + String(i) + " has address " + addressToStr(deviceAddress) + "\n";
		} else {
			s += "found ghost device at " + String(i) + "\n";
		}
	}
	return s;
}

TTemperatureSensor getTemperatureSensor(uint8_t idx) {
	return temperatureSensors[idx];
}

uint8_t getTemperatureSensorsCount() {
	return temperatureSensorsCount;
}

JsonDocument getTempSensorState(TTemperatureSensor &ts) {
	JsonDocument doc;

	doc["id"] = ts.sensorId;
	doc["class"] = "temp";
	// doc["data"]["name"] = ts.name;
	char tmp[6];
	sprintf(tmp, "%0.2f", ts.measuredValue);
	doc["data"]["value"] = tmp;
	doc["data"]["direction"] = ts.direction;
	doc["data"]["min"] = ts.minValue;
	doc["data"]["max"] = ts.maxValue;
	doc["data"]["critical"] = ts.criticalState;
	return doc;
}

JsonDocument getTempSensorState(int idx) {
	TTemperatureSensor ts = getTemperatureSensor(idx);
	return getTempSensorState(ts);
}

/**
 * Store the measured value of a temperature sensor
 */
void storeTemperature(uint8_t idx, float value) {
	// Serial.printf("device %s temperature changed from %0.2f to %0.2f\n", temperatureSensors[idx].name, temperatureSensors[idx].measuredValue, value);
	if (value > temperatureSensors[idx].measuredValue) { temperatureSensors[idx].direction = 1; }
	if (value < temperatureSensors[idx].measuredValue) { temperatureSensors[idx].direction = -1; }
	temperatureSensors[idx].measuredValue = value;
	temperatureSensors[idx].criticalState = value > temperatureSensors[idx].maxValue;
}

/**
 * Save temperature of the sensor identified by its address
 */
void storeTemperature(DeviceAddress addr, float value) {
	String deviceAddress = addressToStr(addr);
//  Serial.printf("store temperature %0.2f for device %s\n", value, deviceAddress.c_str());
	for (uint8_t i = 0; i < getTemperatureSensorsCount(); i++) {
		if (strcmp(temperatureSensors[i].address, deviceAddress.c_str()) == 0) {
			storeTemperature(i, value);
			break;
		}
	}
}

/**
 * Save temperature of the sensor identified by its name
 */
void storeTemperature(const char *name, float value) {
	for (uint8_t i = 0; i < getTemperatureSensorsCount(); i++) {
		if (strcmp(temperatureSensors[i].name, name) == 0) {
			storeTemperature(i, value);
			break;
		}
	}
}

unsigned long temperatureConversionWait(uint8_t resolution) {
	uint8_t res = resolution;
	if (res < 9 || res > 12) { res = 12; }
	if (res == 9) { return 94; }
	else if (res == 10) { return 188; }
	else if (res == 11) { return 375; }
	return 750;
}

/**
 * Return measured temperature of the sensor at the given index
 */
double getSensorTemperature(uint8_t idx) {
	return temperatureSensors[idx].measuredValue;
}

double getSensorTemperatureByName(const char *name) {
	for (uint8_t i = 0; i < getTemperatureSensorsCount(); i++) {
		if (strcmp(temperatureSensors[i].name, name) == 0) {
			return temperatureSensors[i].measuredValue;
		}
	}
	return -1;
}

/**
 * Return minimum temperature of the sensor at the given index
 */
//float TMin(uint8_t idx) {
//  return temperatureSensors[idx].minValue;
//}

/**
 * Return maximum temperature of the sensor at the given index
 */
//float TMax(uint8_t idx) {
//  return temperatureSensors[idx].maxValue;
//}
