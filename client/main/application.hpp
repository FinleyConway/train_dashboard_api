#pragma once

#include <sdkconfig.h>

#include "system_bus.hpp"

#include "networking/wifi/wifi.hpp"
#include "networking/wifi/provisioning/provisioning.hpp"

#include "tasks/tcp_manager_task.hpp"
#include "tasks/motor_task.hpp"
#include "tasks/nfc_task.hpp"
#include "tasks/train_controller_task.hpp"

#include "common/api/types.hpp"

namespace client {
    class application_t {
    public:
        application_t() : 
            m_tcp_manager_task(m_bus, this, on_server_acknowledgement),
            m_motor_task(m_bus),
            m_nfc_task(m_bus),
            m_train_controller_task(m_bus) 
        {
        }

        void run() {
            // setup wifi and attempt to conenct to the network
            static client::wifi_t wifi;

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

            m_tcp_manager_task.init();
            m_motor_task.init();
            m_nfc_task.init();
            m_train_controller_task.init();
        }

    private:
        static void on_wifi_prov_response(bool has_connected) {
            if (has_connected) {
                // restart the esp to take advantage of nvs
                esp_restart();
            }

            // TODO: Provide fail indication through buzzer or lights
        }

        static void on_server_acknowledgement(void* ctx, common::esp_id_t id) {
            auto* app = static_cast<application_t*>(ctx);

            app->m_bus.train_id = id;

            ESP_LOGI(c_tag, "Server ack, assigned id: %hu", id);
        }

    private:
        static constexpr const char* c_tag = "application";

        system_bus_t m_bus;
        tcp_manager_task_t m_tcp_manager_task;
        motor_task_t m_motor_task;
        nfc_task_t m_nfc_task;
        train_controller_task_t m_train_controller_task;
    };
}