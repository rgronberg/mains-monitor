/*
 * mains-monitor: MainsMonitor.hpp
 */

#ifndef __MAINSMONITOR_HPP__
#define __MAINSMONITOR_HPP__

#include <EmonLib.h>

#include "EmonConfig.hpp"

class MainsMonitor
{
private:
    /***** Pins and constants *****/
    const unsigned int SENSOR_PIN = A0;
    const unsigned int SENSOR_SELECT_PIN = 4;
    const unsigned int NUMBER_OF_SAMPLES = 559;
    const unsigned long POLLING_PERIOD = 250;   // ms
    const double POLLING_PERIOD_SECONDS = 0.25; // s
    const unsigned long RESET_PERIOD = 30000;   // ms

    /***** EmonLib instances *****/
    EnergyMonitor emon1;
    EnergyMonitor emon2;

    /***** System configuration *****/
    EmonConfig *emon_config;

    /***** Monitoring variables *****/
    double nominal_voltage = 0.0;               // volts
    double sensor_1_instant_watts   = 0.0;      // watts
    double sensor_2_instant_watts   = 0.0;      // watts
    double daily_kWh = 0.0;                     // kilowatt-hour
    double monthly_kWh = 0.0;                   // kilowatt-hour

    /***** Timing variables *****/
    bool wait_for_ntp = true;
    unsigned long last_process_time = 0;        // ms
    unsigned long last_reset_check = 0;         // ms

    /***** Internal functions *****/
    void time_is_set(bool from_sntp);
    void update_kWh_counters();
    void reset_kWh_counters();

public:
    MainsMonitor(EmonConfig &emon_config);
    void begin();
    void update_calibration();
    void process();
    double sensor_1_watts();
    double sensor_2_watts();
    double watts();
    double get_daily_kWh();
    double get_monthly_kWh();
};

#endif // __MAINSMONITOR_HPP__