//mains-monitor.ino

#include <ESP8266WiFi.h>

#define PRINT_DEBUG_MESSAGES
#define PRINT_HTTP
#include <ThingSpeak.h>

#include "MainsMonitor.hpp"

#define SENSOR_PIN A0
#define SENSOR_1_CAL 62.75
#define SENSOR_2_CAL 62.75

MainsMonitor mainsMonitor;

WiFiClient  client;

const char * myWriteAPIKey = "UNREALKEY";


unsigned long last_report_time = 0;
const unsigned long REPORT_PERIOD = 60 * 1000;  // One minute

double watt_seconds_to_kwatt_hours(double watt_seconds) {
    return watt_seconds / 3600000;
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println();

    mainsMonitor.begin();

    WiFi.begin("SSID", "PASSWORD");

    ThingSpeak.begin(client);  // Initialize ThingSpeak

      if(WiFi.status() != WL_CONNECTED){
    Serial.println("Attempting to connect to SSID: ");
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(1000);     
    } 
    Serial.println("\nConnected.");
  }
}

void loop()
{
    // Every 0.25 seconds, calculate watt-seconds
    // Integrate every reading over the course of one mintue
    mainsMonitor.process();


    // If one second has elapsed, report values and reset
    if ((millis() - last_report_time) > REPORT_PERIOD) {
        last_report_time = millis();
        Serial.print("Ingegration duration (ms): ");
        Serial.println(REPORT_PERIOD);
        Serial.print("Sensor 1 (watt-seconds): ");
        Serial.print(mainsMonitor.sensor_1_watt_seconds());
        Serial.print(", Sensor 2 (watt-seconds): ");
        Serial.println(mainsMonitor.sensor_2_watt_seconds());
        double total_kwatt_hours = watt_seconds_to_kwatt_hours(mainsMonitor.sensor_1_watt_seconds()) +
                                   watt_seconds_to_kwatt_hours(mainsMonitor.sensor_2_watt_seconds());
        Serial.print("Total KW hours: ");
        Serial.println(total_kwatt_hours, 16);


        Serial.println(WiFi.status() == WL_CONNECTED);

        int x = ThingSpeak.writeField(2030469, 1, (float)total_kwatt_hours, myWriteAPIKey);
        if(x == 200) {
            Serial.println("Channel update successful.");
        }
        else {
            Serial.println("Problem updating channel. HTTP error code " + String(x));
        }


        mainsMonitor.reset_integration();
    }

    // Report watt-minutes used in the past minute and reset integration counters
}
