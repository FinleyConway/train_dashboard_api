#include <esp_err.h>
#include <cJSON.h>

#include "networking/wifi/wifi.hpp"
#include "networking/wifi//wifi_provisioning.hpp"
#include "esp_https_server.h"

#include "task_events/tcp_send_event.hpp"
#include "tasks/tcp_task.hpp"

// https://developer.espressif.com/blog/2025/06/basic_http_server/

extern "C" void app_main() {
    static client::wifi_t wifi;

    ESP_ERROR_CHECK(wifi.connect_from_nvs());

    if (!wifi.wait_connection()) {
        client::wifi_provisioning_t wifi_prov(wifi);

        ESP_ERROR_CHECK(wifi_prov.start_softap("esp", "", 1));

        g_event_group = xEventGroupCreate();
        xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);
        httpd_handle_t server = start_webserver();

        static const httpd_uri_t wifi_cred_uri = {
            .uri       = "/wifi_cred",               
            .method    = HTTP_POST,        
            .handler   = wifi_cred_post_handler, 
            .user_ctx  = NULL            
        };

        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &wifi_cred_uri));

        bool try_connection = true;
        while (try_connection) {
            EventBits_t bits = xEventGroupWaitBits(
                g_event_group,
                g_received_bit | g_fail_bit,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY
            );

            if (bits & g_received_bit) {
                ESP_ERROR_CHECK(wifi_prov.try_station_connect(g_ssid, g_password));

                if (wifi_prov.wait_connection()) {
                    ESP_ERROR_CHECK(wifi_prov.stop());
                    stop_webserver(server);

                    try_connection = false;

                    ESP_LOGW("HTTP", "Success!");

                    esp_restart();
                }
                else {
                    xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);

                    ESP_LOGW("HTTP", "Failed to connect, retrying...");
                }
            }
            else {
                xEventGroupClearBits(g_event_group, g_received_bit | g_fail_bit);

                ESP_LOGW("HTTP", "Failed to ssid and password, retrying...");
            }
        }
    }

    ESP_LOGI("YEYY", "WORKS??");
}