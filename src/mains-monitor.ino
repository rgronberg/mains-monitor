//mains-monitor.ino

#include <ESP8266WiFi.h>

#define NTP_SERVER "us.pool.ntp.org"
#include <coredecls.h>  // settimeofday_cb() callback
#include <time.h>
#include <TZ.h>

// #define PRINT_DEBUG_MESSAGES
// #define PRINT_HTTP
#include <ThingSpeak.h>

#include "MainsMonitor.hpp"

MainsMonitor mainsMonitor;

WiFiClient  client;

// Timekeeping globals
time_t now;
tm tm;

bool wait_for_ntp = true;

const char * myWriteAPIKey = "UNREALKEY";

unsigned long last_report_time = 0;

const unsigned long REPORT_PERIOD = 30 * 1000;  // Thirty seconds, ms

double daily_KWh = 0.0;
double monthly_KWh = 0.0;

double watt_seconds_to_kwatt_hours(double watt_seconds) {
    return watt_seconds / 3600000;
}

void time_is_set(bool from_sntp) {
    if (wait_for_ntp) {
      Serial.print(F("time was sent! from_sntp=")); Serial.println(from_sntp);
    }
    wait_for_ntp = false;
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.println();

    WiFi.begin("SSID", "PASSWORD");

    if (WL_CONNECTED != WiFi.status()){
        Serial.println("Attempting to connect to SSID: ");
        while (WL_CONNECTED != WiFi.status()){
          Serial.print(".");
          delay(1000);
        }
        Serial.println("\nConnected.");
    }

    // Set NTP callback
    settimeofday_cb(time_is_set);

    // Configure timezone and NTP server
    configTime(TZ_America_Los_Angeles, NTP_SERVER);

    while (wait_for_ntp) {
        Serial.println("Waiting for time to be set from NTP");
        delay(1000);
    }

    // Initialize energy monitor
    mainsMonitor.begin();

    // Initialize ThingSpeak
    ThingSpeak.begin(client);
}

void loop()
{
    // Every POLLING_PERIOD, calculate watt-seconds
    // Integrate every reading over the course of REPORT_PERIOD
    mainsMonitor.process();

    // If REPORT_PERIOD has elapsed, report values and reset integration
    if ((millis() - last_report_time) > REPORT_PERIOD) {
        last_report_time = millis();
        Serial.println();

        // Reset the daily_kWh count at midnight. Might end up being reset twice
        // Due to the 30s update frequencey
        time(&now);
        localtime_r(&now, &tm);
        if (0 == tm.tm_hour && 0 == tm.tm_min) {
            Serial.println("Midnight, reset daily_KWh");
            daily_KWh = 0.0;
            // Reset monthly_kWh integration on the 1st day of the month at midnight
            if (16 == tm.tm_mday) {
                Serial.println("First of the month, reset monthly_KWh");
                monthly_KWh = 0.0;
            }
            Serial.println("#################################################");
        }

        double total_kwatt_hours = watt_seconds_to_kwatt_hours(mainsMonitor.sensor_1_watt_seconds()) +
                                   watt_seconds_to_kwatt_hours(mainsMonitor.sensor_2_watt_seconds());
        daily_KWh += total_kwatt_hours;
        monthly_KWh += total_kwatt_hours;

        Serial.print("Sensor 1 (watt-seconds): ");
        Serial.print(mainsMonitor.sensor_1_watt_seconds());
        Serial.print(", Sensor 2 (watt-seconds): ");
        Serial.println(mainsMonitor.sensor_2_watt_seconds());
        Serial.print("Total KW hours: ");
        Serial.println(total_kwatt_hours, 16);
        Serial.print("daily_KWh: ");
        Serial.println(daily_KWh, 16);
        Serial.print("monthly_KWh: ");
        Serial.println(monthly_KWh, 16);

        // Report fields to ThingSpeak
        ThingSpeak.setField(1, (float)daily_KWh);
        ThingSpeak.setField(2, (float)monthly_KWh);
        ThingSpeak.setField(3, (float)mainsMonitor.watts());
        ThingSpeak.setField(4, (float)mainsMonitor.watts());
        int status_code = ThingSpeak.writeFields(0, myWriteAPIKey);
        if(200 == status_code) {
            Serial.println("Channel update successful.");
        }
        else {
            Serial.println("Problem updating channel. HTTP error code " + String(status_code));
        }

        // Reset polling integration counters
        mainsMonitor.reset_integration();
    }
}
