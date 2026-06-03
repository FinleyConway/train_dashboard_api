#include <esp_err.h>

#include "networking/wifi/wifi.hpp"
#include "networking/wifi//wifi_provisioning.hpp"

#include "task_events/tcp_send_event.hpp"
#include "tasks/tcp_task.hpp"


extern "C" void app_main() {
    static client::wifi_t wifi;
    client::wifi_provisioning_t wifi_prov(wifi);

    ESP_ERROR_CHECK(wifi_prov.start_softap("esp", "", 1));
    ESP_ERROR_CHECK(wifi_prov.start_sta_provisioning("ssid", "password"));

    if (wifi_prov.wait_connection()) {
        ESP_ERROR_CHECK(wifi_prov.stop());

        ESP_ERROR_CHECK(wifi.connect("ssid", "password"));

        if (wifi.wait_connection()) {
            client::tcp_send_event_t::create(10);
            client::tcp_task_t::init(2);
        }
    }
}