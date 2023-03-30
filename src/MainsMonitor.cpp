/*
 * mains-monitor: MainsMonitor.cpp
 */

#include "MainsMonitor.hpp"

void MainsMonitor::begin(double calibration, double voltage) {
    pinMode(SENSOR_SELECT_PIN, OUTPUT);
    calibrate(calibration);
    nominal_voltage = voltage;
}

void MainsMonitor::calibrate(double calibration) {
    emon1.current(SENSOR_PIN, calibration);
    emon2.current(SENSOR_PIN, calibration);
}

void MainsMonitor::voltage(double voltage) {
    nominal_voltage = voltage;
}

void MainsMonitor::process() {
    if ((millis() - last_process_time) > POLLING_PERIOD) {
        last_process_time = millis();
        digitalWrite(SENSOR_SELECT_PIN, LOW);
        sensor_1_instant_watts = emon1.calcIrms(559);
        sensor_1_integration += sensor_1_instant_watts * POLLING_PERIOD_SECONDS;
        digitalWrite(SENSOR_SELECT_PIN, HIGH);
        sensor_2_instant_watts = emon2.calcIrms(559);
        sensor_2_integration += sensor_2_instant_watts * POLLING_PERIOD_SECONDS;
        integration_duration += POLLING_PERIOD;
    }
}

double MainsMonitor::sensor_1_watt_seconds() {
    return sensor_1_integration * nominal_voltage;
}

double MainsMonitor::sensor_2_watt_seconds() {
    return sensor_2_integration * nominal_voltage;
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

void MainsMonitor::reset_integration() {
    sensor_1_integration = 0.0;
    sensor_2_integration = 0.0;
    integration_duration = 0;
}
