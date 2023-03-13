/*
 * mains-monitor: MainsMonitor.cpp
 */

#include "MainsMonitor.hpp"

void MainsMonitor::begin() {
    pinMode(SENSOR_SELECT_PIN, OUTPUT);
    emon1.current(SENSOR_PIN, SENSOR_1_CAL);
    emon2.current(SENSOR_PIN, SENSOR_2_CAL);
}

void MainsMonitor::process() {
    if ((millis() - last_process_time) > POLLING_PERIOD) {
        last_process_time = millis();
        digitalWrite(SENSOR_SELECT_PIN, LOW);
        sensor_1_integration += emon1.calcIrms(1480) * 0.25;
        digitalWrite(SENSOR_SELECT_PIN, HIGH);
        sensor_2_integration += emon2.calcIrms(1480) * 0.25;
        integration_duration += POLLING_PERIOD;
    }
}

double MainsMonitor::sensor_1_watt_seconds() {
    return sensor_1_integration * BASELINE_VOLTAGE;
}

double MainsMonitor::sensor_2_watt_seconds() {
    return sensor_2_integration * BASELINE_VOLTAGE;
}

void MainsMonitor::reset_integration() {
    sensor_1_integration = 0.0;
    sensor_2_integration = 0.0;
    integration_duration = 0;
}
