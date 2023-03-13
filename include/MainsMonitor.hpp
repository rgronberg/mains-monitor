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
    const double BASELINE_VOLTAGE = 118.0;
    const double SENSOR_1_CAL = 62.75;
    const double SENSOR_2_CAL = 62.75;
    const unsigned long POLLING_PERIOD = 250;   // ms

    /***** EmonLib instances *****/
    EnergyMonitor emon1;
    EnergyMonitor emon2;

    /***** Monitoring variables *****/
    double sensor_1_integration = 0.0;          // watt-second
    double sensor_2_integration = 0.0;          // watt-second

    /***** Timing variables *****/
    unsigned long last_process_time = 0;        // ms
    unsigned long integration_duration = 0;     // ms
public:
    void begin();
    void process();
    double sensor_1_watt_seconds();
    double sensor_2_watt_seconds();
    void reset_integration();
};

#endif // __MAINSMONITOR_HPP__