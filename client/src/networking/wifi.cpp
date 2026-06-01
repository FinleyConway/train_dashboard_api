#include "networking/wifi.hpp"
#include "wifi.hpp"

namespace client {

    wifi_t::wifi_t() {
        ESP_ERROR_CHECK(init());
    }

    wifi_t::~wifi_t() {
        if (deinit() != ESP_OK) {
            ESP_LOGE(c_tag, "Failed to deinitialize Wi-Fi");
        }
    }

    esp_err_t wifi_t::connect(const char* ssid, const char* password, bool is_ap) {
        if (is_ap) {
            return start_ap(ssid, password);
        }
            
        return start_sta(ssid, password);
    }

    esp_err_t wifi_t::disconnect() {
        return esp_wifi_disconnect();
    }

    esp_err_t wifi_t::init() {
        // TODO: This could be in its own section/file?
        // initialize nvs
        esp_err_t ret = nvs_flash_init();

        // erase and reinit if full/corrupted or is nvs new format
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }

        s_wifi_event_group = xEventGroupCreate();

        ret = esp_netif_init();
        if (ret != ESP_OK) {
            ESP_LOGE(c_tag, "Failed to initialize TCP/IP network stack");
            return ret;
        }

        ret = esp_event_loop_create_default();
        if (ret != ESP_OK) {
            ESP_LOGE(c_tag, "Failed to create default event loop");
            return ret;
        }

        // Wi-Fi stack configuration parameters
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        return ret;
    }

    esp_err_t wifi_t::start_sta(const char* ssid, const char* password)
    {
        ESP_ERROR_CHECK(esp_wifi_set_default_wifi_sta_handlers());

        s_netif = esp_netif_create_default_wifi_sta();
        if (s_netif == nullptr) {
            ESP_LOGE(c_tag, "Failed to create default WiFi STA interface");
            return ESP_FAIL;
        }

        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event,
            nullptr,
            &s_wifi_event_handler
        ));

        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &ip_event,
            nullptr,
            &s_ip_event_handler
        ));

        wifi_config_t wifi_config = {};

        strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strlcpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        EventBits_t bits = xEventGroupWaitBits(
            s_wifi_event_group, 
            c_connected_bit | c_fail_bit,
            pdFALSE, pdFALSE, 
            portMAX_DELAY
        );

        if (bits & c_connected_bit) {
            ESP_LOGI(c_tag, "Connected to Wi-Fi network: %s", wifi_config.sta.ssid);
            return ESP_OK;
        } 
        else if (bits & c_fail_bit) {
            ESP_LOGE(c_tag, "Failed to connect to Wi-Fi network: %s", wifi_config.sta.ssid);
            return ESP_FAIL;
        }

        ESP_LOGE(c_tag, "Unexpected Wi-Fi error");

        return ESP_FAIL;
    }

    esp_err_t wifi_t::start_ap(const char* ssid, const char* password)
    {
        esp_netif_create_default_wifi_ap();

        wifi_config_t wifi_config = {};

        strlcpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
        strlcpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));

        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.ap.max_connection = 4;

        if (strlen(password) == 0 && wifi_config.ap.authmode != WIFI_AUTH_OWE) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

        return esp_wifi_start();
    }

    esp_err_t wifi_t::deinit()
    {
        if (s_wifi_event_group != nullptr) {
            vEventGroupDelete(s_wifi_event_group);
        }

        esp_err_t ret = esp_wifi_stop();
        if (ret == ESP_ERR_WIFI_NOT_INIT) {
            ESP_LOGE(c_tag, "Wi-Fi stack not initialized");
            return ret;
        }

        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_netif));
        esp_netif_destroy(s_netif);

        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, s_ip_event_handler));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, s_wifi_event_handler));

        return ESP_OK;
    }

    void wifi_t::ip_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        ESP_LOGI(c_tag, "Handling IP event, event code 0x%" PRIx32, event_id);

        switch (event_id) {

            case (IP_EVENT_STA_GOT_IP): {
                ip_event_got_ip_t *event_ip = (ip_event_got_ip_t *)event_data;
                ESP_LOGI(c_tag, "Got IP: " IPSTR, IP2STR(&event_ip->ip_info.ip));
                s_retry_count = 0;
                xEventGroupSetBits(s_wifi_event_group, c_connected_bit);
                break;
            }

            case (IP_EVENT_STA_LOST_IP): {
                ESP_LOGI(c_tag, "Lost IP");
                break;
            }

            case (IP_EVENT_GOT_IP6): {
                ip_event_got_ip6_t* event_ip6 = (ip_event_got_ip6_t*)event_data;
                ESP_LOGI(c_tag, "Got IPv6: " IPV6STR, IPV62STR(event_ip6->ip6_info.ip));
                s_retry_count = 0;
                xEventGroupSetBits(s_wifi_event_group, c_connected_bit);
                break;
            }

            default: {
                ESP_LOGI(c_tag, "IP event not handled");
                break;
            }

        }
    }

    void wifi_t::wifi_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        ESP_LOGI(c_tag, "Handling Wi-Fi event, event code 0x%" PRIx32, event_id);

        switch (event_id) {

            case (WIFI_EVENT_WIFI_READY): {
                ESP_LOGI(c_tag, "Wi-Fi ready");
                break;
            }

            case (WIFI_EVENT_SCAN_DONE): {
                ESP_LOGI(c_tag, "Wi-Fi scan done");
                break;
            }

            case (WIFI_EVENT_STA_START): {
                ESP_LOGI(c_tag, "Wi-Fi started, connecting to AP...");
                esp_wifi_connect();
                break;
            }

            case (WIFI_EVENT_STA_STOP): {
                ESP_LOGI(c_tag, "Wi-Fi stopped");
                break;
            }
            
            case (WIFI_EVENT_STA_CONNECTED): {
                ESP_LOGI(c_tag, "Wi-Fi connected");
                break;
            }

            case (WIFI_EVENT_STA_DISCONNECTED): {
                ESP_LOGI(c_tag, "Wi-Fi disconnected");
                if (s_retry_count < c_retry_attempts) {
                    ESP_LOGI(c_tag, "Retrying to connect to Wi-Fi network...");
                    esp_wifi_connect();
                    s_retry_count++;
                } 
                else {
                    ESP_LOGI(c_tag, "Failed to connect to Wi-Fi network");
                    xEventGroupSetBits(s_wifi_event_group, c_fail_bit);
                }
                break;
            }

            case (WIFI_EVENT_STA_AUTHMODE_CHANGE): {
                ESP_LOGI(c_tag, "Wi-Fi authmode changed");
                break;
            }

            
            case (WIFI_EVENT_AP_STACONNECTED): {
                break;
            }

            
            case (WIFI_EVENT_AP_STADISCONNECTED): {
                break;
            }


            default: {
                ESP_LOGI(c_tag, "Wi-Fi event not handled");
                break;
            }

        }
    }
}