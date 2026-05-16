#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>

#include "networking/wifi_config.hpp"

namespace client {
    enum class wifi_status_t {
        success,
        failure
    };

    class wifi_boot_t {
    public:
        static wifi_status_t connect() {
            constexpr const char* TAG = "WIFI_BOOT";
            constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;
            constexpr EventBits_t WIFI_FAIL_BIT = BIT1;

            EventGroupHandle_t events = xEventGroupCreate();

            if (!events) return wifi_status_t::failure;

            s_retry_count = 0;

            wifi_config_t cfg = get_wifi_config();

            // initialize the esp network interface
            ESP_ERROR_CHECK(esp_netif_init()); 

            // initialize default esp event loop
            ESP_ERROR_CHECK(esp_event_loop_create_default());

            // create wifi station in the wifi driver
            esp_netif_create_default_wifi_sta();

            // setup wifi station with the default wifi configuration
            wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

            // set the wifi controller to be a station
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

            // set the wifi config
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));

            // set up event for wifi 
            esp_event_handler_instance_t wifi_handler;
            esp_event_handler_instance_t ip_handler;

            ESP_ERROR_CHECK(esp_event_handler_instance_register(
                WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                &wifi_event,
                events,
                &wifi_handler
            ));

            ESP_ERROR_CHECK(esp_event_handler_instance_register(
                IP_EVENT,
                IP_EVENT_STA_GOT_IP,
                &ip_event,
                events,
                &ip_handler
            ));

            ESP_LOGI(TAG, "Starting WiFi...");

            // start the wifi driver
            ESP_ERROR_CHECK(esp_wifi_start());

            // evaulate event bits
            EventBits_t bits = xEventGroupWaitBits(
                events,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY
            );

            // unregister events
            esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler);
            esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_handler);
            vEventGroupDelete(events);

            return (bits & WIFI_CONNECTED_BIT) ? wifi_status_t::success : wifi_status_t::failure;
        }

    private:
        static void wifi_event(void* arg, esp_event_base_t base, int32_t id, void*) {
            auto events = static_cast<EventGroupHandle_t>(arg);
            constexpr int WIFI_FAIL_BIT = BIT1;

            if (base != WIFI_EVENT) return;

            if (id == WIFI_EVENT_STA_START) {
                esp_wifi_connect();
            }
            else if (id == WIFI_EVENT_STA_DISCONNECTED) {
                s_retry_count++;

                if (s_retry_count < c_max_retry_count) {
                    esp_wifi_connect();
                } 
                else {
                    xEventGroupSetBits(events, WIFI_FAIL_BIT);
                }
            }
        }

        static void ip_event(void* arg, esp_event_base_t base, int32_t id, void* data) {
            auto events = static_cast<EventGroupHandle_t>(arg);
            constexpr int WIFI_CONNECTED_BIT = BIT0;

            if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
                xEventGroupSetBits(events, WIFI_CONNECTED_BIT);
            }
        }

    private:
        inline static uint8_t s_retry_count = 0;
        static constexpr uint8_t c_max_retry_count = 10;
    };
}