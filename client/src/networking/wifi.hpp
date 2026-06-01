#pragma once

#include <cstdint>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>

// https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/#part-2-using-the-wi-fi-apis
// https://developer.espressif.com/blog/2025/04/soft-ap-tutorial/#starting-the-soft-ap

namespace client {
    class wifi_t {  
    public:
        wifi_t();

        ~wifi_t();

        esp_err_t connect(const char* ssid, const char* password, bool is_ap);

        esp_err_t disconnect();

    private:
        esp_err_t init();

        esp_err_t start_sta(const char* ssid, const char* passwor);

        esp_err_t start_ap(const char* ssid, const char* passwor);

        esp_err_t deinit();

        static void ip_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        static void wifi_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    private:
        static constexpr const char* c_tag = "WIFI";
        static constexpr uint8_t c_connected_bit = BIT0;
        static constexpr uint8_t c_fail_bit = BIT1;

        inline static uint8_t s_retry_count = 0;
        static constexpr uint8_t c_retry_attempts = 3;

        inline static esp_netif_t* s_netif = nullptr;
        inline static esp_event_handler_instance_t s_ip_event_handler;
        inline static esp_event_handler_instance_t s_wifi_event_handler;

        inline static EventGroupHandle_t s_wifi_event_group = nullptr;
    };
}