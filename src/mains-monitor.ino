//mains-monitor.ino

#include <WiFiManager.h>
// #include <ESP8266WiFi.h>

#include "EmonConfig.hpp"



// #define PRINT_DEBUG_MESSAGES
// #define PRINT_HTTP
#include <ThingSpeak.h>

#include "MainsMonitor.hpp"
#include "WebServer.hpp"

EmonConfig emon_config;
MainsMonitor mainsMonitor(emon_config);
WiFiManager wifiManager;
WebServer webServer(emon_config, mainsMonitor, wifiManager);

WiFiClient  client;

const char * myWriteAPIKey = "UNREALKEY";

unsigned long last_report_time = 0;

const unsigned long REPORT_PERIOD = 30 * 1000;  // Thirty seconds, ms

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println();

    emon_config.begin();
    Serial.println(emon_config.read_config());
    emon_config.load_config();

    WiFi.begin("SSID", "PASSWORD");

    if (WL_CONNECTED != WiFi.status()){
        Serial.println("Attempting to connect to SSID: ");
        while (WL_CONNECTED != WiFi.status()){
          Serial.print(".");
          delay(1000);
        }
        Serial.println("\nConnected.");
    }

    // Initialize energy monitor
    mainsMonitor.begin();

    // Initialize
    webServer.begin();

    // Initialize ThingSpeak
    ThingSpeak.begin(client);
    last_report_time = millis();
}

void loop()
{
    // Handle web server traffic
    webServer.handleClient();

    // Every POLLING_PERIOD, calculate watt-seconds
    // Integrate every reading over the course of REPORT_PERIOD
    mainsMonitor.process();

    // If REPORT_PERIOD has elapsed, report values
    if ((millis() - last_report_time) > REPORT_PERIOD) {
        last_report_time = millis();
        Serial.println();

        Serial.print("daily_KWh:\t");
        Serial.println(mainsMonitor.get_daily_kWh(), 16);
        Serial.print("monthly_KWh:\t");
        Serial.println(mainsMonitor.get_monthly_kWh(), 16);

        // Report fields to ThingSpeak
        ThingSpeak.setField(1, (float)mainsMonitor.get_daily_kWh());
        ThingSpeak.setField(2, (float)mainsMonitor.get_monthly_kWh());
        ThingSpeak.setField(3, (float)mainsMonitor.watts());
        ThingSpeak.setField(4, (float)mainsMonitor.watts());
        int status_code = ThingSpeak.writeFields(0, myWriteAPIKey);
        if(200 == status_code) {
            Serial.println("Channel update successful.");
        }
        else {
            Serial.println("Problem updating channel. HTTP error code " + String(status_code));
        }
    }
}
