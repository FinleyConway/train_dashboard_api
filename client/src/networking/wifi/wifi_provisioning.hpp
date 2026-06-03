#pragma once

#include <esp_err.h>
#include <esp_log.h>

#include "networking/wifi/wifi.hpp"

namespace client {
    class wifi_provisioning_t {
    public:
        explicit wifi_provisioning_t(wifi_t& wifi) : m_wifi(wifi) {}

        esp_err_t start_softap(const char* ssid, const char* password, uint8_t max_connections) {
            ESP_LOGI("WIFI_PROV", "Starting provisioning");

            ESP_ERROR_CHECK(m_wifi.set_mode(wifi_mode_t::sta_ap));
            ESP_ERROR_CHECK(m_wifi.set_softap_config(ssid, password, max_connections));

            return m_wifi.start();
        }

        esp_err_t start_sta_provisioning(const char* ssid, const char* password) {
            ESP_LOGI("WIFI_PROV", "Starting station provisioning");

            m_wifi.stop();
            ESP_ERROR_CHECK(m_wifi.set_mode(wifi_mode_t::sta_ap));
            ESP_ERROR_CHECK(m_wifi.set_sta_config(ssid, password));

            return m_wifi.start();
        }

        bool wait_connection() {
            ESP_LOGI("WIFI_PROV", "Trying to connect");

            return m_wifi.wait_connection();
        }

        esp_err_t stop() {
            ESP_LOGI("WIFI_PROV", "Stopping provisioning");

            m_wifi.disconnect();
            m_wifi.reset_retry();

            return m_wifi.set_mode(wifi_mode_t::none);
        }

    private:
        static constexpr const char* c_tag = "WIFI_PROV";

        wifi_t& m_wifi;
    };
}