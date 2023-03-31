/*
 * mains-monitor: MainsMonitor.cpp
 */

#include <coredecls.h>  // settimeofday_cb() callback
#include <time.h>

#include "MainsMonitor.hpp"

double watt_seconds_to_kWh(double watt_seconds) {
    return watt_seconds / 3600000;
}

void MainsMonitor::time_is_set(bool from_sntp) {
    if (wait_for_ntp) {
      Serial.print(F("time was sent! from_sntp=")); Serial.println(from_sntp);
    }
    wait_for_ntp = false;
}

void MainsMonitor::update_kWh_counters() {
    double sensor_1_current = 0.0;
    double sensor_1_watt_seconds = 0.0;
    double sensor_2_current = 0.0;
    double sensor_2_watt_seconds = 0.0;
    double cycle_kWh = 0.0;

    // Calculate current readings from sensors and update kWh used
    if ((millis() - last_process_time) > POLLING_PERIOD) {
        last_process_time = millis();

        // Get current sensor values
        digitalWrite(SENSOR_SELECT_PIN, LOW);
        sensor_1_current = emon1.calcIrms(NUMBER_OF_SAMPLES);
        sensor_1_instant_watts = sensor_1_current * emon_config->nominal_voltage;
        sensor_1_watt_seconds = sensor_1_instant_watts * POLLING_PERIOD_SECONDS;
        digitalWrite(SENSOR_SELECT_PIN, HIGH);
        sensor_2_current = emon2.calcIrms(NUMBER_OF_SAMPLES);
        sensor_2_instant_watts = sensor_2_current * emon_config->nominal_voltage;
        sensor_2_watt_seconds = sensor_2_instant_watts * POLLING_PERIOD_SECONDS;

        // Update daily and monthly kWh used
        cycle_kWh = watt_seconds_to_kWh(sensor_1_watt_seconds + sensor_2_watt_seconds);
        daily_kWh += cycle_kWh;
        monthly_kWh += cycle_kWh;

        // Serial.printf("sensor_1_current: %.2f\tsensor_2_current: %.2f\n",
        //               sensor_1_current, sensor_2_current);
    }
}

void MainsMonitor::reset_kWh_counters() {
    time_t now;
    struct tm tm;

    // Only attempt to reset kWh counters every 30 seconds
    if ((millis() - last_reset_check) > RESET_PERIOD) {
        // Reset the daily_kWh count at midnight. Might end up being reset twice
        // Due to the 30s update frequencey
        time(&now);
        localtime_r(&now, &tm);
        if (0 == tm.tm_hour && 0 == tm.tm_min) {
            Serial.println("Midnight, reset daily_KWh");
            daily_kWh = 0.0;
            // Reset monthly_kWh integration on the 1st day of the month at midnight
            if (16 == tm.tm_mday) {
                Serial.println("First of the month, reset monthly_KWh");
                monthly_kWh = 0.0;
            }
        }
    }
}

MainsMonitor::MainsMonitor(EmonConfig &emon_config) {
    this->emon_config = &emon_config;
}

void MainsMonitor::begin() {
    // Initialize analog MUX and EmonLib instances
    pinMode(SENSOR_SELECT_PIN, OUTPUT);
    update_calibration();

    // Set NTP callback
    settimeofday_cb([this](bool from_sntp) {
        time_is_set(from_sntp);
    });

    // Configure timezone and NTP server
    configTime(emon_config->time_zone, emon_config->ntp_server);

    // Wait for time from NTP server
    while (wait_for_ntp) {
        Serial.println("Waiting for time to be set from NTP");
        delay(1000);
    }

    last_process_time = last_reset_check = millis();
}

void MainsMonitor::update_calibration() {
    emon1.current(SENSOR_PIN, emon_config->calibration);
    emon2.current(SENSOR_PIN, emon_config->calibration);
}

void MainsMonitor::process() {
    update_kWh_counters();
    reset_kWh_counters();
}

double MainsMonitor::sensor_1_watts() {
    return sensor_1_instant_watts;
}
double MainsMonitor::sensor_2_watts() {
    return sensor_2_instant_watts;
}

double MainsMonitor::watts() {
    return sensor_1_watts() + sensor_2_watts();
}

double MainsMonitor::get_daily_kWh() {
    return daily_kWh;
}

double MainsMonitor::get_monthly_kWh() {
    return monthly_kWh;
}
