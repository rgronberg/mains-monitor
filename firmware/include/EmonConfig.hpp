/*
 * mains-monitor: EmonConfig.hpp
 */

#ifndef __EMONCONFIG_HPP__
#define __EMONCONFIG_HPP__

class EmonConfig
{
private:
    const char *config_filename = "/emon_config.txt";
public:
    /***** Config values *****/
    char hostname[32];
    char api_key[32];
    unsigned long channel;
    double calibration;
    double nominal_voltage;
    int reset_date;
    char time_zone[48];
    char ntp_server[48];

    /***** Config functions *****/
    void begin();
    void load_config();
    String read_config();
    void save_config();
    void reset_config();
};

#endif // __EMONCONFIG_HPP__
