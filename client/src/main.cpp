#include "networking/wifi.hpp"

extern "C" void app_main() {
    client::wifi_status_t status = client::wifi_boot_t::connect();

    if (status == client::wifi_status_t::failure) {
        ESP_LOGI("ESP_MAIN", "Failed to associate to AP, dying...");

        return;
    }
}