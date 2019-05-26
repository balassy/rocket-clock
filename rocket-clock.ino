// Platform libraries.
#include <Arduino.h>     // To add IntelliSense for platform constants.
#include <ESP8266WiFi.h> // Required dependency for the NTPtimeESP library.

// Third-party libraries.
#include <WiFiManager.h>  // To manage network configuration and connection.
#include "NTPtimeESP.h"   // To get the current time through NTP.

// My classes.
#include "status-led.h"

// Read configuration and secrets.
#include "config.h"

WiFiManager wifiManager;
NTPtime ntpClient;
StatusLed nightLed;
StatusLed dayLed;

void setup() {
  initSerial();
  initLeds();
  initNetwork();
  initTimeServerConnection();

  Serial.println("Setup completed.");
}

void initSerial() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing serial connection DONE.");
}

void initLeds() {
  nightLed.setPin(PIN_NIGHT_LED);
  dayLed.setPin(PIN_DAY_LED);

  nightLed.turnOn();
  dayLed.turnOn();
  delay(500);
  nightLed.turnOff();
  dayLed.turnOff();
}

void initNetwork() {
  Serial.printf("Initializing connection to the network with MAC address %s using WiFiManager (SSID: %s)...\n", WiFi.macAddress().c_str(), WIFI_AP_SSID);
  wifiManager.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Serial.printf("WiFi connected. IP address: %s, MAC address: %s\n", WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
}

void initTimeServerConnection() {
  Serial.printf("Initializing time server connection to %s...\n", NTP_SERVER_HOST);
  ntpClient.init(NTP_SERVER_HOST);
}

void loop() {
  strDateTime currentTime = getCurrentTime();
  bool isNowDayTime = isDayTime(currentTime);
  if (isNowDayTime) {
    switchToDayTime();
  }
  else {
    switchToNightTime();
  }

  delay(60000);
}

void switchToNightTime() {
  Serial.println("Switching to night time...");
  nightLed.turnOn();
  dayLed.turnOff();
}

void switchToDayTime() {
  Serial.println("Switching to day time...");
  nightLed.turnOff();
  dayLed.turnOn();
}

strDateTime getCurrentTime() {
  Serial.print("Getting current time...");

  ntpClient.setRecvTimeout(59);
  strDateTime currentTime = ntpClient.getNTPtime(TIME_ZONE, DST_ADJUSTMENT);

  if(currentTime.valid){
    ntpClient.printDateTime(currentTime);
    return currentTime;
  } else {
    Serial.println("Getting current time failed, waiting and trying again...");
    delay(100);  // Adding this delay make the function work 100% reliable on the subsequent try.
    getCurrentTime();
  }
}

bool isDayTime(strDateTime currentTime) {
  byte currentHour = currentTime.hour;
  return currentHour >= DAYTIME_BEGIN_HOUR && currentHour <= DAYTIME_END_HOUR;
}
