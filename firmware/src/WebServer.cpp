/*
 * mains-monitor: WebServer.cpp
 */

#include <LittleFS.h>

#include "WebServer.hpp"

WebServer::WebServer(EmonConfig &emonConfig, MainsMonitor &mainsMonitor, WiFiManager& wifiManager) {
    this->emonConfig = &emonConfig;
    this->mainsMonitor = &mainsMonitor;
    this->wifiManager = &wifiManager;

    handleNotFound();
    handleStatic();
    handleIndex();
    handleSettings();
    handleReset();
}

void WebServer::begin() {
    LittleFS.begin();
    serverReady = true;
    server.begin();
}

void WebServer::handleClient() {
    bool executeReset = false;

    if ((resetFlag || mainsMonitor->should_reset() || hostnameChanged) && resetTime == 0) {
        resetTime = millis();
    }

    if (resetTime != 0 && ((millis() - resetTime) > RESET_DELAY)) {
        executeReset = true;
    }

    if (executeReset && (resetFlag || mainsMonitor->should_reset())) {
        emonConfig->reset_config();
        wifiManager->resetSettings();
        ESP.restart();
    }

    if (executeReset && hostnameChanged) {
        ESP.restart();
    }

    if (serverReady) {
        server.handleClient();
    }
}

void WebServer::handleNotFound() {
    server.onNotFound([this]() {
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plane", "");
    });
}

void WebServer::handleStatic() {
    server.on("/favicon.ico", [this]() {
        File favicon = LittleFS.open("/favicon.ico", "r");
        if (favicon) {
            server.streamFile(favicon, "image/x-icon");
            favicon.close();
        }
        else {
            server.send(503, "text/plain", "server error");
        }
    });

    server.on("/static/main.js", [this]() {
        File mainJs = LittleFS.open("/main.js", "r");
        if (mainJs) {
            server.streamFile(mainJs, "application/javascript");
            mainJs.close();
        }
        else {
            server.send(503, "text/plain", "server error");
        }
    });

    server.on("/static/bootstrap.min.css", [this]() {
        File bootstrapCss = LittleFS.open("/bootstrap.min.css.gz", "r");
        if (bootstrapCss) {
            server.streamFile(bootstrapCss, "text/css");
            bootstrapCss.close();
        }
        else {
            server.send(503, "text/plain", "server error");
        }
    });

    server.on("/static/bootstrap.min.css.map", [this]() {
        File bootstrapMap = LittleFS.open("/bootstrap.min.css.map.gz", "r");
        if (bootstrapMap) {
            server.streamFile(bootstrapMap, "text/json");
            bootstrapMap.close();
        }
        else {
            server.send(503, "text/plain", "server error");
        }
    });


}

void WebServer::handleIndex() {
    server.on("/", [this]() {
        File indexHtml = LittleFS.open("/index.html", "r");
        if (indexHtml) {
            String indexHtmlString = indexHtml.readString();
            indexHtmlString.replace("%daily_kWh%", String(mainsMonitor->get_daily_kWh()));
            indexHtmlString.replace("%monthly_kWh%", String(mainsMonitor->get_monthly_kWh()));
            indexHtmlString.replace("%watts%", String(mainsMonitor->watts()));
            server.send(200, "text/html", indexHtmlString);
            indexHtml.close();
        }
        else {
            server.send(503, "text/plain", "server error");
       }
    });
}

void WebServer::handleSettings() {
    server.on("/settings", [this]() {
        // Update settings in config and redirect to inex
        if (server.args()) {
            // Update values in config and save to disk
            strlcpy(emonConfig->hostname, server.arg("hostname").c_str(), sizeof(emonConfig->hostname));
            strlcpy(emonConfig->api_key, server.arg("api_key").c_str(), sizeof(emonConfig->api_key));
            emonConfig->channel = server.arg("channel").toInt();
            emonConfig->calibration = server.arg("calibration").toDouble();
            emonConfig->nominal_voltage = server.arg("nominal_voltage").toDouble();
            emonConfig->reset_date = server.arg("reset_date").toInt();
            strlcpy(emonConfig->time_zone, server.arg("time_zone").c_str(), sizeof(emonConfig->time_zone));
            strlcpy(emonConfig->ntp_server, server.arg("ntp_server").c_str(), sizeof(emonConfig->ntp_server));
            emonConfig->save_config();

            // New hostname differs, if so reboot device
            if (WiFi.hostname() != emonConfig->hostname) {
                Serial.printf("new hostname: %s, old hostname %s\n", emonConfig->hostname, WiFi.hostname().c_str());
                Serial.println("Saving new hostname to config and rebooting");
                hostnameChanged = true;
            }

            // Update mainsMonitor with new calibration
            mainsMonitor->update_calibration();

            // Redirect to index
            server.sendHeader("Location", "/", true);
            server.send(302, "text/plane", "");
        }
        // Display settings form
        else {
            File settingsHtml = LittleFS.open("/settings.html", "r");
            if (settingsHtml) {
                String settingsHtmlString = settingsHtml.readString();
                settingsHtmlString.replace("%hostname%", emonConfig->hostname);
                settingsHtmlString.replace("%api_key%", emonConfig->api_key);
                settingsHtmlString.replace("%channel%", String(emonConfig->channel));
                settingsHtmlString.replace("%calibration%", String(emonConfig->calibration));
                settingsHtmlString.replace("%nominal_voltage%", String(emonConfig->nominal_voltage));
                settingsHtmlString.replace("%reset_date%", String(emonConfig->reset_date));
                settingsHtmlString.replace("%time_zone%", emonConfig->time_zone);
                settingsHtmlString.replace("%ntp_server%", emonConfig->ntp_server);
                server.send(200, "text/html", settingsHtmlString);
                settingsHtml.close();
            }
            else {
                server.send(503, "text/plain", "server error");
            }
        }
    });
}

void WebServer::handleReset() {
    this->server.on("/reset", [this]() {
        this->server.send(200, "text/plain", "All settings and WiFi credentials reset and rebooting...");
        resetFlag = true;
    });
}
