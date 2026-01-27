#ifndef __DS18B20UTILS_H
#define __DS18B20UTILS_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

// temperature sensor
struct TTemperatureSensor {
	bool isReady;
	const char *address;
	const char *name;
	const char *sensorId;
	float measuredValue;
	float minValue;
	float maxValue;
	bool criticalState;
	short direction; // 1 -> up, -1 -> down
};

String addressToStr(DeviceAddress addr);
String enumTemperatureSensors(DallasTemperature &dtSensors);
TTemperatureSensor getTemperatureSensor(uint8_t idx);
uint8_t getTemperatureSensorsCount();
JsonDocument getTempSensorState(TTemperatureSensor &ts);
JsonDocument getTempSensorState(int idx);
void storeTemperature(uint8_t idx, float value);
void storeTemperature(DeviceAddress addr, float value);
void storeTemperature(const char *name, float value);
unsigned long temperatureConversionWait(uint8_t resolution);

double getSensorTemperature(uint8_t idx);
double getSensorTemperatureByName(const char *name);
//float TMin(uint8_t idx);
//float TMax(uint8_t idx);

#endif
