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
        bool serverReady = false;
        int resetFlag = false;
        ESP8266WebServer server;
        EmonConfig *emonConfig;
        MainsMonitor *mainsMonitor;
        WiFiManager *wifiManager;

        void handleNotFound();
        void handleStatic();
        void handleIndex();
        void handleSettings();
        void handleReset();

        // String templateProcessor(const String &templateString);

    public:
        WebServer(EmonConfig &emonConfig, MainsMonitor &mainsMonitor, WiFiManager &wifiManager);
        void begin();
        void handleClient();
};

#endif // __WEBSERVER_HPP__
