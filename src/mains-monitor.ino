//mains-monitor.ino

#include <ESP8266NetBIOS.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>

ESP8266NetBIOS netBIOS;
WiFiClient  client;

#include "EmonConfig.hpp"
#include "MainsMonitor.hpp"
#include "WebServer.hpp"

EmonConfig emon_config;
MainsMonitor mainsMonitor(emon_config);
WiFiManager wifiManager;
WebServer webServer(emon_config, mainsMonitor, wifiManager);

unsigned long last_report_time = 0;
const unsigned long REPORT_PERIOD = 30 * 1000;  // Thirty seconds, ms

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println();

    emon_config.begin();
    Serial.println(emon_config.read_config());
    emon_config.load_config();

    WiFi.mode(WIFI_STA);
    WiFi.hostname(emon_config.hostname);
    netBIOS.begin(emon_config.hostname);

    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConfigPortalTimeout(120);
    wifiManager.setConnectTimeout(30);

    if (wifiManager.autoConnect(emon_config.hostname, "defaultpassword")) {
        webServer.begin();
    }

    // Initialize energy monitor
    mainsMonitor.begin();

    // Initialize ThingSpeak
    ThingSpeak.begin(client);
    last_report_time = millis();
}

void loop()
{
    if (wifiManager.process()) {
        webServer.begin();
    }

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

        if (strnlen(emon_config.api_key, sizeof(emon_config.api_key)) > 0 && emon_config.channel != 0) {
            // Report fields to ThingSpeak
            ThingSpeak.setField(1, (float)mainsMonitor.get_daily_kWh());
            ThingSpeak.setField(2, (float)mainsMonitor.get_monthly_kWh());
            ThingSpeak.setField(3, (float)mainsMonitor.watts());
            ThingSpeak.setField(4, (float)mainsMonitor.watts());
            int status_code = ThingSpeak.writeFields(emon_config.channel, emon_config.api_key);
            if(200 == status_code) {
                Serial.println("Channel update successful.");
            }
            else {
                Serial.println("Problem updating channel. HTTP error code " + String(status_code));
            }
        }
        else {
            Serial.println("ThingSpeak settings not configured");
        }
    }
}
