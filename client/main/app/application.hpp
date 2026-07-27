#pragma once

#include <sdkconfig.h>

#include "app/system_bus.hpp"

#include "wifi/wifi.hpp"
#include "wifi/provisioning/provisioning.hpp"

#include "tcp/tasks/tcp_manager_task.hpp"
#include "battery/tasks/battery_level_task.hpp"
#include "motor/tasks/motor_task.hpp"
#include "nfc/tasks/nfc_task.hpp"
#include "train_controller/train_controller_task.hpp"

#include "common/api/types.hpp"

#include "train_controller/passive_buzzer.hpp"

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

namespace client {
    class application_t {
    public:
        application_t() : 
            m_tcp_manager_task(m_bus, this, on_server_acknowledgement),
            m_battery_task(m_bus),
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
        }

    private:
        static void on_wifi_prov_response(bool has_connected) {
            if (has_connected) {
                // restart the esp to take advantage of nvs
                esp_restart();
            }

            auto buzzer = passive_buzzer_t::create(GPIO_NUM_13, LEDC_CHANNEL_1);

            if (!buzzer.has_value()) {
                ESP_LOGE(c_tag, "Can't play fail wifi connect indicator!");
                return;
            }

            ESP_ERROR_CHECK(buzzer->set_tone_delay(5000, pdMS_TO_TICKS(1000)));
        }

        static void on_server_acknowledgement(void* ctx, common::esp_id_t id) {
            auto* app = static_cast<application_t*>(ctx);

            app->m_bus.train_id = id;

            ESP_LOGI(c_tag, "Server ack, assigned id: %hu", id);

            app->m_battery_task.init();
            app->m_motor_task.init();
            app->m_nfc_task.init();
            app->m_train_controller_task.init();
        }

    private:
        static constexpr const char* c_tag = "application";

        system_bus_t m_bus;
        tcp_manager_task_t m_tcp_manager_task;

        battery_level_task_t m_battery_task;
        motor_task_t m_motor_task;
        nfc_task_t m_nfc_task;
        train_controller_task_t m_train_controller_task;
    };
}