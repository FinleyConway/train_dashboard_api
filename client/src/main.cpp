#include <nvs_flash.h>

#include "networking/wifi/wifi.hpp"

#include "task_events/tcp_send_event.hpp"
#include "tasks/tcp_task.hpp"


extern "C" void app_main() {
    static client::wifi_t wifi;

    esp_err_t ret = wifi.start_provisioning("esp", "", 1);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to connect to network");
    }

    wifi.start_sta_provisioning("ssid", "password");

    wifi.stop_provisioning();

    wifi.connect("ssid", "password");

    client::tcp_send_event_t::create(10);
    client::tcp_task_t::init(2);
}