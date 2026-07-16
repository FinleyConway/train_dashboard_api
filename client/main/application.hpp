#pragma once

#include <sdkconfig.h>

#include "system_bus.hpp"

#include "networking/wifi/wifi.hpp"
#include "networking/wifi/provisioning/provisioning.hpp"

#include "common/api/types.hpp"

namespace client {
    class application_t {
    public:
        void run() {
            // setup wifi and attempt to conenct to the network
            client::wifi_t wifi;

            ESP_ERROR_CHECK(wifi.connect_from_nvs());

            if (!wifi.wait_connection()) {
                // provide feedback for failing to connect to network
                on_wifi_prov_response(false); 

                client::provisioning_t wifi_prov(wifi);

                // provide and wait for provided network creds through AP
                wifi_prov.start(
                    CONFIG_WIFI_AP_SSID,
                    CONFIG_WIFI_AP_PASSWORD,
                    CONFIG_WIFI_AP_MAX_CONNECTIONS
                );
                wifi_prov.wait_connection(on_wifi_prov_response);
            }

            ESP_LOGI(c_tag, "Connected to network, waiting for server response...");

            tcp_manager_task_t::init(on_server_acknowledgement);
        }

    private:
        void on_wifi_prov_response(bool has_connected) {
            if (has_connected) {
                // restart the esp to take advantage of nvs
                esp_restart();
            }

            // TODO: Provide fail indication through buzzer or lights
        }

        void on_server_acknowledgement(common::esp_id_t id) {

        }

    private:
        static constexpr const char* c_tag = "application";

        system_bus_t m_bus;
    };
}