/*
 * mains-monitor: EmonConfig.cpp
 */

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <TZ.h>

#include "EmonConfig.hpp"

void EmonConfig::begin() {
    LittleFS.begin();
}

void EmonConfig::load_config() {
    File file = LittleFS.open(config_filename, "r");
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to read file, using default configuration...");
    }
    else {
        Serial.println("Reading config from file...");
    }

    // Read from config file if opened, else set defaults
    snprintf(hostname, sizeof(hostname), doc["hostname"] | "emon-%d", ESP.getChipId());
    strlcpy(api_key, doc["api_key"] | "", sizeof(api_key));
    strlcpy(channel, doc["channel"] | "", sizeof(channel));
    calibration = doc["calibration"] | 62.75;
    nominal_voltage = doc["nominal_voltage"] | 120.0;
    if (!doc["time_zone"]) {
        strncpy_P(time_zone, TZ_America_Los_Angeles, sizeof(time_zone));
        time_zone[sizeof(time_zone)-1] = 0;
        Serial.println("TZ set to default.");
    }
    else {
        strlcpy(time_zone, doc["time_zone"], sizeof(time_zone));
        Serial.println("TZ set by config.");
    }
    strlcpy(ntp_server, doc["ntp_server"] | "us.pool.ntp.org", sizeof(ntp_server));

    file.close();
}

String EmonConfig::read_config() {
    File file = LittleFS.open(config_filename, "r");

    if (file) {
        return file.readString();
    }

    return String("failed to open config file...");
}

void EmonConfig::save_config () {
    File file = LittleFS.open(config_filename, "w");
    StaticJsonDocument<512> doc;

    if (!file) {
        Serial.println("failed to create config file");
        return;
    }

    doc["hostname"] = hostname;
    doc["api_key"] = api_key;
    doc["channel"] = channel;
    doc["calibration"] = calibration;
    doc["nominal_voltage"] = nominal_voltage;
    doc["time_zone"] = time_zone;
    doc["ntp_server"] = ntp_server;

    if (serializeJsonPretty(doc, file) == 0) {
        Serial.println("Failed to write config file");
    }

    file.close();
}