/*
 * mains-monitor: MainsMonitor.hpp
 */

#ifndef __MAINSMONITOR_HPP__
#define __MAINSMONITOR_HPP__

#include <EmonLib.h>

class MainsMonitor
{
private:
    /***** Pins and constants *****/
    const unsigned int SENSOR_PIN = A0;
    const unsigned int SENSOR_SELECT_PIN = 4;
    const unsigned long POLLING_PERIOD = 250;   // ms
    const double POLLING_PERIOD_SECONDS = 0.25; // s

    /***** EmonLib instances *****/
    EnergyMonitor emon1;
    EnergyMonitor emon2;

    /***** Monitoring variables *****/
    double nominal_voltage = 0.0;               // volts
    double sensor_1_integration = 0.0;          // watt-second
    double sensor_2_integration = 0.0;          // watt-second
    double sensor_1_instant_watts   = 0.0;      // watts
    double sensor_2_instant_watts   = 0.0;      // watts

    /***** Timing variables *****/
    unsigned long last_process_time = 0;        // ms
    unsigned long integration_duration = 0;     // ms
public:
    void begin(double calibration, double voltage);
    void calibrate(double calibration);
    void voltage(double voltage);
    void process();
    double sensor_1_watt_seconds();
    double sensor_2_watt_seconds();
    double sensor_1_watts();
    double sensor_2_watts();
    double watts();
    void reset_integration();
};

#endif // __MAINSMONITOR_HPP__