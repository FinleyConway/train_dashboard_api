#pragma once

#include <cstring>

#include <esp_wifi.h>

namespace client {
    class wifi_config_t {
    public:
        static esp_err_t init_softap(const char* ssid, const char* password, uint8_t max_connections) {
            ::wifi_config_t cfg = {};

            strlcpy((char*)cfg.ap.ssid, ssid, sizeof(cfg.ap.ssid));
            strlcpy((char*)cfg.ap.password, password, sizeof(cfg.ap.password));

            cfg.ap.ssid_len = strlen(ssid);
            cfg.ap.channel = 1;
            cfg.ap.max_connection = max_connections;
            cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
            cfg.ap.pmf_cfg.required = false;

            if (strlen(password) == 0) {
                cfg.ap.authmode = WIFI_AUTH_OPEN;
            }

            return esp_wifi_set_config(WIFI_IF_AP, &cfg);
        }

        static esp_err_t init_station(const char* ssid, const char* password) {
            ::wifi_config_t cfg = {};

            strlcpy((char*)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid));
            strlcpy((char*)cfg.sta.password, password, sizeof(cfg.sta.password));

            return esp_wifi_set_config(WIFI_IF_STA, &cfg);
        }
    };
}