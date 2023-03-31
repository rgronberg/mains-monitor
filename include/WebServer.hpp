/*
 * mains-monitor: WebServer.hpp
 */

#ifndef __WEBSERVER_HPP__
#define __WEBSERVER_HPP__

#include <WiFiManager.h>

#include "EmonConfig.hpp"
#include "MainsMonitor.hpp"

class WebServer
{
    private:
        const unsigned long RESET_DELAY = 5000;

        bool serverReady = false;
        bool resetFlag = false;
        bool hostnameChanged = false;
        unsigned long resetTime = 0;

        ESP8266WebServer server;
        EmonConfig *emonConfig;
        MainsMonitor *mainsMonitor;
        WiFiManager *wifiManager;

        void handleNotFound();
        void handleStatic();
        void handleIndex();
        void handleSettings();
        void handleReset();

    public:
        WebServer(EmonConfig &emonConfig, MainsMonitor &mainsMonitor, WiFiManager &wifiManager);
        void begin();
        void handleClient();
};

#endif // __WEBSERVER_HPP__
