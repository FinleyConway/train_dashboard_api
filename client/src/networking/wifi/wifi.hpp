#pragma once

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <freertos/event_groups.h>

#include "networking/wifi/wifi_config.hpp"

namespace client {
    enum class wifi_mode_t {
        none,
        sta,
        sta_ap
    };

    class wifi_t {
    public:
        wifi_t() {
            // TODO: This could be in its own section/file?
            // initialize nvs
            esp_err_t ret = nvs_flash_init();
            // erase and reinit if full/corrupted or is nvs new format
            if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
            }
            ESP_ERROR_CHECK(ret);

            m_event_group = xEventGroupCreate();
            if (m_event_group == nullptr) {
                ESP_LOGE("WIFI", "Can't create event group!");
                abort();
            }

            ESP_ERROR_CHECK(esp_netif_init());
            ESP_ERROR_CHECK(esp_event_loop_create_default());

            ESP_ERROR_CHECK(esp_event_handler_instance_register(
                WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                &wifi_event,
                this,
                &m_wifi_event_handler
            ));
            ESP_ERROR_CHECK(esp_event_handler_instance_register(
                IP_EVENT,
                ESP_EVENT_ANY_ID,
                &ip_event,
                this,
                &m_ip_event_handler
            ));

            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        }
    
        ~wifi_t() {
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_deinit());

            if (m_ap_netif != nullptr) {
                esp_netif_destroy(m_ap_netif);
                m_ap_netif = nullptr;
            }

            if (m_sta_netif != nullptr) {
                esp_netif_destroy(m_sta_netif);
                m_sta_netif = nullptr;
            }

            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_unregister(
                WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                m_wifi_event_handler
            ));
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_unregister(
                IP_EVENT,
                ESP_EVENT_ANY_ID,
                m_ip_event_handler
            ));

            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_delete_default());

            vEventGroupDelete(m_event_group);
        }

        esp_err_t connect(const char* ssid, const char* password) {
            ESP_ERROR_CHECK(set_mode(wifi_mode_t::sta));

            handle_create_netif(wifi_mode_t::sta);

            ESP_ERROR_CHECK(wifi_config_t::init_station(ssid, password));

            m_retry_count = 0;
            xEventGroupClearBits(
                m_event_group,
                c_connected_bit | c_fail_bit
            );

            ESP_ERROR_CHECK(esp_wifi_start());

            EventBits_t bits = xEventGroupWaitBits(
                m_event_group,
                c_connected_bit | c_fail_bit,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY
            );

            if (bits & c_connected_bit) {
                return ESP_OK;
            }

            return ESP_FAIL;
        }


        esp_err_t start_provisioning(const char* ssid, const char* password, uint8_t max_connections) {
            ESP_ERROR_CHECK(set_mode(wifi_mode_t::sta_ap));

            handle_create_netif(wifi_mode_t::sta_ap);

            ESP_ERROR_CHECK(wifi_config_t::init_softap(ssid, password, max_connections));

            ESP_LOGI(c_tag, "Starting wifi provisioning");

            m_is_softap_provisioning = true;

            return esp_wifi_start();
        }

        esp_err_t start_sta_provisioning(const char* ssid, const char* password) {
            ESP_LOGI(c_tag, "Starting wifi sta provisioning");

            m_is_softap_provisioning = false; 

            ESP_ERROR_CHECK(esp_wifi_stop());
            ESP_ERROR_CHECK(set_mode(wifi_mode_t::sta_ap));
            ESP_ERROR_CHECK(wifi_config_t::init_station(ssid, password));

            m_retry_count = 0;
            xEventGroupClearBits(
                m_event_group,
                c_connected_bit | c_fail_bit
            );

            ESP_ERROR_CHECK(esp_wifi_start());

            EventBits_t bits = xEventGroupWaitBits(
                m_event_group,
                c_connected_bit | c_fail_bit,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY
            );

            if (bits & c_connected_bit) {
                ESP_LOGI(c_tag, "Wifi sta provisioning connected");

                return ESP_OK;
            }

            return ESP_FAIL;
        }

        esp_err_t stop_provisioning() {
            // stop ap_sta
            ESP_ERROR_CHECK(esp_wifi_disconnect());

            // wake up and stop sta
            xEventGroupSetBits(m_event_group, c_fail_bit);

            m_retry_count = 0;

            ESP_LOGI(c_tag, "Stopping wifi provisioning");

            return set_mode(wifi_mode_t::none);
        }

    private:
        void handle_create_netif(wifi_mode_t mode) {
            switch (mode) {
                case (wifi_mode_t::sta): {
                    if (m_sta_netif == nullptr) {
                        m_sta_netif = esp_netif_create_default_wifi_sta();
                    }
                    break;
                }

                case (wifi_mode_t::sta_ap): {
                    if (m_sta_netif == nullptr) {
                        m_sta_netif = esp_netif_create_default_wifi_sta();
                    }
                    if (m_ap_netif == nullptr) {
                        m_ap_netif = esp_netif_create_default_wifi_ap();
                    }
                    break;
                }

                default: break;
            }
        }

        esp_err_t set_mode(wifi_mode_t mode) {
            if (m_mode == mode) {
                return ESP_OK;
            }

            // must stop the wifi before switching modes
            if (m_mode != wifi_mode_t::none) {
                ESP_ERROR_CHECK(esp_wifi_stop());
            }

            m_mode = mode;

            switch (mode) {
                case (wifi_mode_t::none):   return esp_wifi_set_mode(WIFI_MODE_NULL);
                case (wifi_mode_t::sta):    return esp_wifi_set_mode(WIFI_MODE_STA);
                case (wifi_mode_t::sta_ap): return esp_wifi_set_mode(WIFI_MODE_APSTA);
                default:                    return ESP_FAIL;
            }
        }

        static void wifi_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            auto* self = static_cast<wifi_t*>(arg);

            switch (event_id) {
                case (WIFI_EVENT_STA_START): {
                    ESP_LOGI(c_tag, "Station started");
                    ESP_ERROR_CHECK(esp_wifi_connect());
                    break;
                }

                case (WIFI_EVENT_STA_DISCONNECTED): {
                    ESP_LOGW(c_tag, "Station disconneceted");

                    if (self->m_is_softap_provisioning) return;

                    if (self->m_retry_count < c_retry_attempts) {
                        self->m_retry_count++;

                        esp_wifi_connect();

                        return;
                    }

                    ESP_LOGE(c_tag, "Connection failed");
                    xEventGroupSetBits(self->m_event_group, c_fail_bit);
                    break;
                }

                default: break;
            }
        }

        static void ip_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            auto* self = static_cast<wifi_t*>(arg);

            switch (event_id) {
                case (IP_EVENT_STA_GOT_IP): {
                    self->m_retry_count = 0;
                    xEventGroupSetBits(self->m_event_group, c_connected_bit);
                    break;
                }

                default: break;
            }
        }

    private:
        static constexpr const char* c_tag = "WIFI";

        wifi_mode_t m_mode = wifi_mode_t::none;

        EventGroupHandle_t m_event_group = nullptr;
        esp_event_handler_instance_t m_wifi_event_handler = nullptr;
        esp_event_handler_instance_t m_ip_event_handler = nullptr;
        static constexpr EventBits_t c_connected_bit = BIT0;
        static constexpr EventBits_t c_fail_bit = BIT1;

        esp_netif_t* m_ap_netif = nullptr;
        esp_netif_t* m_sta_netif = nullptr;

        uint8_t m_retry_count = 0;
        static constexpr uint8_t c_retry_attempts = 3;

        bool m_is_softap_provisioning = false;
    };
}