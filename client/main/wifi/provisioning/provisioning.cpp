#include "wifi/provisioning/provisioning.hpp"

#include <esp_mac.h>

namespace client {
    provisioning_t::provisioning_t(wifi_t& wifi) : m_wifi(wifi) {}

    esp_err_t provisioning_t::start(const char* ap_ssid, const char* ap_password, uint8_t max_connections) {
        ESP_LOGI(c_tag, "Starting provisioning");

        esp_err_t ret = start_softap(ap_ssid, ap_password, max_connections);
        if (ret != ESP_OK) {
            ESP_LOGE(c_tag, "Failed to start softap!");
            
            return ret;
        }

        return m_http.try_start();
    }

    esp_err_t provisioning_t::start_softap(const char* ap_ssid, const char* ap_password, uint8_t max_connections) {
        ESP_ERROR_CHECK(m_wifi.set_mode(wifi_mode_t::sta_ap));
        
        char unique_ap_ssid[32];
        create_unique_ap_ssid(ap_ssid, unique_ap_ssid);

        ESP_ERROR_CHECK(m_wifi.set_softap_config(unique_ap_ssid, ap_password, max_connections));

        return m_wifi.start();
    }

    esp_err_t provisioning_t::create_unique_ap_ssid(const char* ap_ssid, char ap_ssid_buffer[32]) {
        constexpr uint8_t max_ssid_size = 32;
        constexpr uint8_t suffix_size = 7; // "-000000"
        constexpr uint8_t max_user_ssid = max_ssid_size - suffix_size;

        if (strnlen(ap_ssid, max_ssid_size) > max_user_ssid) {
            ESP_LOGE(c_tag, "AP SSID too long (max %u chars)", (unsigned)max_user_ssid);

            return ESP_FAIL;
        }

        uint8_t ap_mac[6];

        ESP_ERROR_CHECK(esp_read_mac(ap_mac, ESP_MAC_WIFI_SOFTAP));

        snprintf(
            ap_ssid_buffer, 
            max_ssid_size, 
            "%s-%02X%02X%02X", 
            ap_ssid, ap_mac[3], ap_mac[4], ap_mac[5]
        );

        return ESP_OK;
    }

    esp_err_t provisioning_t::try_station_connect(const char* ssid, const char* password)
    {
        if (m_wifi.get_mode() != wifi_mode_t::sta_ap) return ESP_FAIL;

        ESP_LOGI(c_tag, "Starting station provisioning");

        ESP_ERROR_CHECK(m_wifi.set_sta_config(ssid, password));
        m_wifi.reset_retry();
        
        return esp_wifi_connect();
    }

    esp_err_t provisioning_t::stop() {
        ESP_LOGI(c_tag, "Stopping provisioning");

        m_wifi.disconnect();
        m_wifi.reset_retry();

        return m_wifi.set_mode(wifi_mode_t::none);
    }
}