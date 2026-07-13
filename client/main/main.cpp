#include <esp_err.h>
#include <esp_log.h>
#include <sdkconfig.h>

#include "networking/wifi/wifi.hpp"
#include "networking/wifi/provisioning/provisioning.hpp"

#include "tasks/tcp/tcp_manager_task.hpp"
#include "tasks/motor_task.hpp"
#include "tasks/nfc_task.hpp"

void connection_handle(bool has_connected) {
    if (has_connected) {
        ESP_LOGI("main", "Connected!");

        // restart the esp to take advantage of nvs
        esp_restart();
    }
    else {
        ESP_LOGI("main", "Given creds werent correct, try again!");
    }
}

extern "C" void app_main() {
    static client::wifi_t wifi;

    ESP_ERROR_CHECK(wifi.connect_from_nvs());

    if (!wifi.wait_connection()) {
        client::provisioning_t wifi_prov(wifi);

        wifi_prov.start(
            CONFIG_WIFI_AP_SSID,
            CONFIG_WIFI_AP_PASSWORD,
            CONFIG_WIFI_AP_MAX_CONNECTIONS
        );
        wifi_prov.wait_connection(connection_handle);
    }

    ESP_LOGI("main", "connected");

    client::tcp_manager_task_t::init([](common::esp_id_t id) {
        client::motor_task_t::init(1);
        client::nfc_task_t::init(1);
    });
}