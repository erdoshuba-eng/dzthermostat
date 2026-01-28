/*

*/

#include <Arduino.h>
#if defined(ESP32)
	#include <WebServer.h>
	#include <HTTPClient.h>
	// #include "mbedtls/md.h"   // HMAC
#elif defined(ESP8266)
// #else
	#include <ESP8266WebServer.h>
	#include <ESP8266HTTPClient.h>
	// #include <bearssl/bearssl_hmac.h>
	// #include <bearssl/bearssl_hash.h>
#endif
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <WiFiUdp.h>

#include "ds18b20_utils.h"
#include "config.h"
#include "device.h"
#include "udp.h"
#include "httpreq.h"
#ifdef HAS_DISPLAY
	// #include "pcd8544_utils.h"
	// TPDC8544Display display;
	#include "ssd1306_091.h"
	SSD1306_091 display;
#endif

extern const char* deviceId;
String ver = "1.0.0";

// --------------- configuration
struct Config {
	char name[16];
	char ssid[20];
	char psw[20];
	char _ip[16];
	char gw[16]; // gateway
	char nm[16]; // netmask
	bool paramsAccepted;
};

Config config;
const char* configFileName = "/config.ini";

// network
bool paramsAccepted = false; // indicates stored wifi credentials are ok
int8_t failedConnectAttempts = 0;
bool networkOk = false; // indicates current internet/network access
unsigned long lastWiFiConnectTrial;
// unsigned long lastLEDToggle;

extern const char* MONITOR_URL;
extern const char* ROOT_CA;
ESP8266WebServer webServer(80);
bool webServerInitialized = false;
unsigned long lastAlivePublished;
// WiFiUDP udpListener;

// temparature sensors
bool canReadTemperature = false; // semaphore for timer
OneWire oneWire(ONE_WIRE_BUS);
// pass our one wire reference to Dallas Temperature
DallasTemperature dtSensors(&oneWire);
Ticker tmrReadTemperature; // timer used to control temperature reading frequency
extern TTemperatureSensor temperatureSensors[];
TThermostat thermostat("th", "Termosztát", deviceId);
static bool tempConversionPending = false;
static unsigned long tempConversionStartMs = 0;
static unsigned long tempConversionWaitMs = 750;

bool displEnabled = true; // will be false if the displw cannot be initialized
bool btnUpPressed = false;
bool btnDownPressed = false;

// void announceTemperatures();
void connectToWiFi();
void doCommunication();
void enableReadTemperature();
void readTemperatures();
void setupTemperatureSensors();
void setupTimers();
void initFS();
void initDevices();
void loadSettings();
void manageSystem();
bool saveSettings();
void serveFile(String);
void setupWebServer();
IPAddress strToIp(const char *);
void updateDisplay();

void setup() {
#ifdef DEBUG
	Serial.begin(9600); // 115200
	delay(1000);
	Serial.println("serial initialized");
#else
	delay(1500);
#endif
	pinMode(PIN_UP, INPUT); // button to rise reference temperature
	pinMode(PIN_DOWN, INPUT); // button to lower reference temperature
	// pinMode(PIN_LED, OUTPUT);
	// digitalWrite(PIN_LED, HIGH);

#ifdef HAS_DISPLAY
	// pinMode(pinDisplayLED, OUTPUT);
	// digitalWrite(pinDisplayLED, LOW);
	// display.init();
	// display.contrast(65);
#ifdef DEBUG
    Serial.println("initialize the display");
#endif
	if (!display.init()) {
		displEnabled = false;
#ifdef DEBUG
    	Serial.println(F("SSD1306 allocation failed, display is disabled"));
#endif
	}
#endif

	setupTemperatureSensors();

	initFS();
	strlcpy(config.name, "_thermostat_", sizeof(config.name));
	loadSettings();

	initDevices();

	connectToWiFi();
	setupWebServer();
	setupTimers();
	udpListen();
	lastAlivePublished = 0;
}

void loop() {
	if (WiFi.getMode() == WIFI_AP) {
		webServer.handleClient(); // HTTP server listening to requests
	} else {
		if (!WiFi.isConnected()) {
			if (millis() - lastWiFiConnectTrial > 20 * 1000) {
				connectToWiFi();
			}
		} else {
			doCommunication();
			// HTTP server listening to requests
			webServer.handleClient();
		}
	}
	readTemperatures();
	yield();
	delay(3);
	manageSystem();
}

void setupTimers() {
	// temperature
	tmrReadTemperature.attach(freqReadTemperature, enableReadTemperature);
	enableReadTemperature();
}

void acceptWiFiParams() {
	if (config.paramsAccepted) { return; }
	config.paramsAccepted = true;
	saveSettings();
}

/**
 * Load the content of the configuration file
 */
void loadSettings() {
	File f = LittleFS.open(configFileName, "r");
	if (!f) {
#ifdef DEBUG
		Serial.println("Failed to open the config file to read.");
#endif
		return;
	}

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, f);
	f.close();
	if (error) {
#ifdef DEBUG
		Serial.print("Failed to read file content: ");
		Serial.println(error.f_str());
#endif
		return;
	}

	strlcpy(config.name, doc["device"]["name"], sizeof(config.name));
	if (!doc["device"]["ip"]) { doc["device"]["ip"] = "192.168.1.15"; }
	strlcpy(config._ip, doc["device"]["ip"], sizeof(config._ip));
	// config.ip = strToIp(doc["device"]["ip"]);
	if (!doc["device"]["gw"]) { doc["device"]["gw"] = ""; }
	strlcpy(config.gw, doc["device"]["gw"], sizeof(config.gw));
	if (!doc["device"]["nm"]) { doc["device"]["nm"] = ""; }
	strlcpy(config.nm, doc["device"]["nm"], sizeof(config.nm));
	if (!doc["device"]["ssid"]) { doc["device"]["ssid"] = ""; }
	strlcpy(config.ssid, doc["device"]["ssid"], sizeof(config.ssid));
	if (!doc["device"]["psw"]) { doc["device"]["psw"] = ""; }
	strlcpy(config.psw, doc["device"]["psw"], sizeof(config.psw));
	config.paramsAccepted = doc["device"]["paramsAccepted"] || false;

#ifdef DEBUG
	Serial.print("device name read: ");
	Serial.println(config.name);
#endif
}

bool saveSettings() {
	// put data into the json document
	JsonDocument doc;
	doc["device"]["name"] = config.name;
	doc["device"]["ip"] = config._ip;
	doc["device"]["gw"] = config.gw;
	doc["device"]["nm"] = config.nm;
	doc["device"]["ssid"] = config.ssid;
	doc["device"]["psw"] = config.psw;
	doc["device"]["paramsAccepted"] = config.paramsAccepted;

  File f = LittleFS.open(configFileName, "w");
	if (!f) {
#ifdef DEBUG
		Serial.println("Failed to open the config file to write.");
#endif
		return false;
	}

	size_t result = serializeJson(doc, f);
	f.close();
#ifdef DEBUG
	if (result == 0) {
		Serial.println("Failed to write data into config file.");
	} else {
		Serial.println("Config written successfully.");
	}
#endif
	return result != 0;
}

void displayLoadingStep(String msg) {
#ifdef DEBUG
	Serial.println(msg);
#endif
}


//*******************
// Network communication
//*******************

void connectToWiFi() {
	int i = 0;
	displayLoadingStep("connecting to WiFi");
/*
	if (!paramsAccepted) {
		if (strcmp(config.name, "_monitor_") == 0 || failedConnectAttempts == 3) {
			// start AP mode
  		WiFi.mode(WIFI_AP); // mode ap
			WiFi.softAP("monitor_config");
			IPAddress ip = WiFi.softAPIP();
			setupWebServer();
#ifdef DEBUG
			Serial.print("AP IP address: ");
			Serial.println(ip);
#endif
			return;
		}
	}*/
//  WiFi.persistent(false);
//  WiFi.setSleepMode(WIFI_NONE_SLEEP, 0);
	WiFi.mode(WIFI_STA); // mode station
	WiFi.setHostname(config.name);
	// WiFi.config(config.ip, strToIp(config.gw), strToIp(config.nm), IPAddress(8, 8, 8, 8));
	WiFi.config(strToIp(config._ip), strToIp(config.gw), strToIp(config.nm), IPAddress(8, 8, 8, 8));
	WiFi.begin(config.ssid, config.psw); // start connecting
	while (WiFi.status() != WL_CONNECTED && i < 30) {
		delay(500);
		i++;
#ifdef DEBUG
    	Serial.print(".");
#endif
	}

#ifdef DEBUG
	Serial.println();
#endif
	networkOk = false;
	if (WiFi.isConnected()) {
		acceptWiFiParams();
		networkOk = true;
		WiFi.setHostname(config.name);
//#ifdef DEBUG
//    Serial.println();
//    Serial.print("WiFi status code: ");
//    Serial.println(WiFi.status());
//#endif
#ifdef DEBUG
		Serial.println("connected to " + String(config.ssid) + "\nIP address is: " + WiFi.localIP().toString() + "\nstrength: " + String(WiFi.RSSI()));
		if (!WiFi.getAutoReconnect()) { Serial.println("auto reconnect is not enabled"); }
#endif
	} else {
		failedConnectAttempts++;
#ifdef DEBUG
		String sNoNetwork = "no network";
		Serial.println(sNoNetwork);
#endif
	}
	lastWiFiConnectTrial = millis();
}


//*******************
// HTML communication
//*******************

IPAddress strToIp(const char * value) {
	char* str_ip = (char *)malloc(16);
	strcpy(str_ip, value);
	uint8_t nr[4];
	char* token;
	char delim[] = ".";
	uint8_t idx = 0;
	token = strsep(&str_ip, delim);
	while (token != NULL) {
		nr[idx++] = atoi(token);
		token = strsep(&str_ip, delim);
	}
	free(str_ip);
	return IPAddress(nr[0], nr[1], nr[2], nr[3]);
}

/**
 * Returns the corresponding content type of the
 *   extension.
 *
 * @param {String} ext - the extension
 * @returns {String} -
*/
String contentType(String ext) {
	if (ext.equalsIgnoreCase("ico")) { return "image/x-icon"; } else
	if (ext.equalsIgnoreCase("html")) { return "text/html"; } else
	if (ext.equalsIgnoreCase("js")) { return "application/javascript"; } else
	if (ext.equalsIgnoreCase("css")) { return "text/css; charset=utf-8"; } else
	if (ext.equalsIgnoreCase("woff2")) { return "application/x-font-woff2"; } else
	{ return ""; }
}

void handleNotFound() {
	webServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

/**
 * Load a file from the file system and return its content as a
 *   response to a request.
*/
void serveFile(String fName) {
	if (!LittleFS.exists("/" + fName)) { handleNotFound(); }
	File f = LittleFS.open("/" + fName, "r");
	if (!f) { handleNotFound(); }
	// split file name into name and extension
	int n = fName.lastIndexOf(".");
	String ext = fName.substring(n + 1);
	webServer.streamFile(f, contentType(ext));
	// int size = f.size();
	// char buf[512];
	// WiFiClient client = webServer.client();

	// String header = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: " + String(size) + "\r\nContent-Type: image/x-icon\r\n\r\n";
	// client.write(header.c_str());

	// while (size > 0) {
	// 	size_t count = std::min((int)(sizeof(buf) - 1), size);
	// 	f.read((uint8_t *)buf, count); // read count bytes from the file into the buffer
	// 	client.write((const char*)buf, count); // send it back to the client
	// 	size -= count; // decrease the amount to read
	// }
}

/**
 * A lambda expression stored in a function pointer type variable
*/
std::function<void()> serveIndex {
	[]() {
		serveFile(WiFi.getMode() == WIFI_AP ? "config.device.html" : "index.html");
	}
};

std::function<void()> serveFavicon {
	[]() {
		serveFile("favicon.ico");
	}
};

void getConfigDevice() {
	JsonDocument doc;

	doc["success"] = true;
	doc["data"]["name"] = config.name;
	doc["data"]["ip"] = config._ip;
	doc["data"]["gw"] = config.gw;
	doc["data"]["nm"] = config.nm;
	doc["data"]["ssid"] = config.ssid;
	doc["data"]["psw"] = config.psw;

	// stringify
	String payload;
	// size_t dataSize = serializeJson(doc, payload);
	serializeJson(doc, payload);

	webServer.send(200, "application/json", payload);
}

void saveConfigDevice() {
	// store data into the config structure (dest, source, size)
	strlcpy(config.name, webServer.arg("name").c_str(), sizeof(config.name));
	strlcpy(config._ip, webServer.arg("ip").c_str(), sizeof(config._ip));
	// config.ip = strToIp(webServer.arg("ip").c_str());
	strlcpy(config.gw, webServer.arg("gw").c_str(), sizeof(config.gw));
	strlcpy(config.nm, webServer.arg("nm").c_str(), sizeof(config.nm));
	strlcpy(config.ssid, webServer.arg("ssid").c_str(), sizeof(config.ssid));
	strlcpy(config.psw, webServer.arg("psw").c_str(), sizeof(config.psw));

	if (saveSettings()) {
		webServer.send(200, "text/html", "<!DOCTYPE html><html><head></head><body><p>Settings saved. Device is restarting.</p></body></html>");
		delay(2000);
		ESP.restart();
	} else {
	// serveFile("config.device.html");
	}
}
/*
JsonDocument getVersionState() {
	JsonDocument doc;
	doc["type"] = "version";
	doc["version"] = ver;
	return doc;
}*/

/**
 * Report the state to the internal web app.
 * /
void sendState() {
	JsonDocument doc;
	JsonArray items = doc.to<JsonArray>();
	items.add(getVersionState());
	items.add(thermostat.getState2());
	// stringify
	String payload;
	/ *size_t dataSize = * /serializeJson(doc, payload);
	webServer.send(200, "application/json", payload);
}*/

void setupWebServer() {
	String subdirectory = "/thermostat";
	displayLoadingStep("setup web server...");
	webServer.onNotFound(handleNotFound);
	webServer.on("", serveIndex);
	webServer.on("/", serveIndex);
	webServer.on(subdirectory + "/favicon.ico", serveFavicon);
	webServer.on(subdirectory + "/jquery-3.7.1.min.js", []() { serveFile("jquery-3.7.1.min.js"); });
	webServer.on(subdirectory + "/bootstrap.min.js", []() { serveFile("bootstrap.min.js"); });
	webServer.on(subdirectory + "/bootstrap.min.css", []() { serveFile("bootstrap.min.css"); });

	if (WiFi.getMode() == WIFI_AP) {
		webServer.on(subdirectory + "/config.device", []() { serveFile("config.device.html"); });
		webServer.on(subdirectory + "/config.device.js", []() { serveFile("config.device.js"); });
		webServer.on(subdirectory + "/getConfig", getConfigDevice);
		webServer.on(subdirectory + "/save/config/device", HTTP_POST, saveConfigDevice);
	} else {
		// webServer.on(subdirectory + "/getState", sendState);
	}

	webServer.begin(); // start the server
}

/**
 * Send a status message to the MQTT broker
 */
void publishStatus() {
	// send the heartbeat to the monitoring server
	// JsonDocument doc;
	// doc["id"] = systemId;
	// JsonArray devices = doc["devices"].to<JsonArray>(); // because is a system
	// devices.add(thermostat.getState());
	// String url = String(MONITOR_URL) + "/api/system/" + systemId + "/heartbeat";
	JsonDocument doc = thermostat.getState();
	String url = String(MONITOR_URL) + "/api/device/" + deviceId + "/heartbeat";
	httpPost(url, doc, {{"X-Device-ID", deviceId}});

	thermostat.getGate().publishStatus();
}

/**
 * The commands are coming from the web ui
 */
void procesDeviceCommand(TDevice &device) {
	String url = String(MONITOR_URL) + "/api/device/" + device._deviceId + "/command/next";
	String response = httpGet(url, {{"X-Device-ID", deviceId}});
	if (response == "") { return; }

	bool cmdProcessed = false;
	String processResult = "";
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, response);
	if (!error) {
		JsonDocument payloadDoc = doc["payload"];
		cmdProcessed = device.processCommand(payloadDoc, processResult);
	}

	// acknowledge the command
	url = String(MONITOR_URL) + "/api/device/" + device._deviceId + "/command/ack";
	JsonDocument ackDoc;
	ackDoc["command_id"] = doc["command_id"];
	ackDoc["success"] = cmdProcessed;
	ackDoc["result"] = processResult;
	httpPost(url, ackDoc, {{"X-Device-ID", deviceId}});
}

/**
 * Read and process commands one by one for every device
 */
void processCommands() {
	procesDeviceCommand(thermostat);
}

void doCommunication() {
	if (millis() - lastAlivePublished < 5 * 1000) { return; }

	processCommands();
	publishStatus();
	lastAlivePublished = millis();
}


//*******************
// File system operations
//*******************

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
	Serial.printf("Listing directory: %s\n", dirname);

	File root = fs.open(dirname, "r");
	if (!root){
		Serial.println("Failed to open directory");
		return;
	}
	if (!root.isDirectory()){
		Serial.println("Not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.print (file.name());
			time_t t= file.getLastWrite();
			struct tm * tmstruct = localtime(&t);
			Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		} else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.print(file.size());
			time_t t= file.getLastWrite();
			struct tm * tmstruct = localtime(&t);
			Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
		}
		file = root.openNextFile();
	}
}

void printFile(const char* filename) {
	// Open file for reading
	File file = LittleFS.open(filename, "r");
	if (!file) {
		Serial.println(F("Failed to read file"));
		return;
	}

	// Extract each characters by one by one
	while (file.available()) {
		Serial.print((char)file.read());
	}
	Serial.println();

	// Close the file
	file.close();
}

/**
 * initialize the file system
 */
void initFS() {
	displayLoadingStep("initialize file system...");
	if (!LittleFS.begin()) {
#ifdef DEBUG
		Serial.println("unable to activate FS");
#endif
		return;
	}
#ifdef DEBUG
	Serial.println("FS is activated");
#endif
	listDir(LittleFS, "/", 1);
}

//*******************
// Temperature sensors handling operations
//*******************

void setupTemperatureSensors() {
	dtSensors.begin(); // start temperature sensors library
	//  9 bit 0.5    precision,  ~94 ms conversion time
	// 10 bit 0.25   precision, ~188 ms conversion time
	// 11 bit 0.125  precision, ~375 ms conversion time
	// 12 bit 0.0625 precision, ~750 ms conversion time
	dtSensors.setResolution(11);
	dtSensors.setWaitForConversion(false);
	tempConversionWaitMs = temperatureConversionWait(dtSensors.getResolution());
#ifdef DEBUG
	Serial.println(enumTemperatureSensors(dtSensors).c_str()); // enumerate the accessible temperature sensors
#endif
}

/**
 * The routine is called by the timer to enable reading the temperature sensors
 */
void enableReadTemperature() {
	canReadTemperature = true;
}

/**
 * Read the temperature measured by the sensors
 */
void readTemperatures() {
	if (!canReadTemperature) { return; }
	unsigned long now = millis();
	uint8_t sensorsCount = dtSensors.getDeviceCount();
	if (sensorsCount < temperatureSensorsCount) {
		// enumerate temperature sensors
#ifdef DEBUG
		Serial.print("temperature sensors: ");
		Serial.println(sensorsCount);
#endif
		dtSensors.begin();
		enumTemperatureSensors(dtSensors);
	}
	if (!tempConversionPending) {
		dtSensors.requestTemperatures();
		tempConversionStartMs = now;
		tempConversionPending = true;
		return;
	}
	if (!dtSensors.isConversionComplete() && now - tempConversionStartMs < tempConversionWaitMs) {
		return;
	}
	DeviceAddress deviceAddress; // temperature sensor address
	for (uint8_t i = 0; i < sensorsCount; i++) {
		if (!dtSensors.getAddress(deviceAddress, i)) { continue; }
		float temperature = dtSensors.getTempC(deviceAddress) - 2.5; // calibration offset
		if (temperature == DEVICE_DISCONNECTED_C || temperature == 85.00) { continue; }
		storeTemperature(deviceAddress, temperature);
	}
	updateDisplay();
	tempConversionPending = false;
	canReadTemperature = false;
}

//*******************
// Devices handling operations
//*******************

/**
 * Load configuration data from the file system
 */
void loadDeviceConfig() {
	thermostat.loadConfig();
	thermostat.getGate().loadConfig();
}

void initDevices() {
	// showLoadingStep("setup devices...");
	// thermostat.getGate().setOpenState(HIGH);
	// thermostat.getGate().setRemote(ipGFSwitch);
	thermostat.getThermometer().setSensor(&temperatureSensors[0]);
	thermostat.getGate().identify("gfswitch", "");
	// woodFurnace.getGate().setGPIO(PIN_PFU);
	// showLoadingStep("loading configuration...", MIDDLE);
	loadDeviceConfig();
	thermostat.getGate().setOpen(false); // instead of controlRemoteGate
	// showLoadingStep("done", END, 500);
}

void detectButtonPress() {
	if (digitalRead(PIN_UP) == HIGH && !btnUpPressed) {
		btnUpPressed = true; // prevent multiple button press detection
		thermostat.adjustRefTemperature(true);
		updateDisplay();
		return;
	}
	if (digitalRead(PIN_DOWN) == HIGH && !btnDownPressed) {
		btnDownPressed = true; // prevent multiple button press detection
		thermostat.adjustRefTemperature(false);
		updateDisplay();
		return;
	}

	if (digitalRead(PIN_UP) == LOW && btnUpPressed) { // the button is released
		btnUpPressed = false;
		return;
	}
	if (digitalRead(PIN_DOWN) == LOW && btnDownPressed) {
		btnDownPressed = false;
		return;
	}
}

void manageSystem() {
	detectButtonPress();
	if (!WiFi.isConnected()) { return; }
	if (thermostat.detectChanges("")) { updateDisplay(); }
}

//*******************
// display operations
//*******************

void updateDisplay() {
#ifdef HAS_DISPLAY
	display.temperature = thermostat.getThermometer().getTemperature();
	display.refTemp = thermostat.getRefTemperature();
	display.wifiConnected = WiFi.status() == WL_CONNECTED;
	display.needsHeating = thermostat.isOn();
	display.wifiStrength = WiFi.RSSI();
	yield();
	display.updateScreen();
#endif
// #ifdef DEBUG
// 	Serial.print("temperature: ");
// 	Serial.println(thermostat.getThermometer().getTemperature());
// #endif
}
