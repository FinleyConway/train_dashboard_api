#pragma once

#include <freertos/FreeRTOS.h>
#include <sdkconfig.h>

#include "task_events/tcp/tcp_send_event.hpp"
#include "task_events/tcp/tcp_event_data.hpp"
#include "task_events/motor_command.hpp"

#include "networking/tcp_client.hpp"

#include "common/messages/motor.hpp"

namespace client {
    class tcp_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            xTaskCreate(
                run,
                "tcp_task_t",
                s_stack_size,
                nullptr,
                tskIDLE_PRIORITY + priority_offset,
                &s_handle
            );
        }
        
        static TaskHandle_t get_handle() {
            return s_handle;
        }

    private:
        static void run(void* parameters) {
            tcp_client_t client;

            register_messages(client);

            while (true) {
                try_connect(client);

                while (client.is_connected()) {
                    if (!handle_send(client)) break;
                    if (!handle_listen(client)) break;
                }
            
                ESP_LOGI(c_tag, "Disconnected");

                client.disconnect();
            }

            vTaskDelete(nullptr);
        }

        static void register_messages(tcp_client_t& client) {
            client.register_receieve_callback<common::esp_init_request_t, &on_init_request>();
            client.register_receieve_callback<common::motor_control_t, &on_motor_control>();
        }

        static void try_connect(tcp_client_t& client) {
            constexpr TickType_t retry_delay = pdMS_TO_TICKS(5000);
            
            ESP_LOGI(c_tag, "Connecting...");

            while (client.try_connect(CONFIG_TCP_TIMEOUT) != tcp_status_t::success) {
                vTaskDelay(retry_delay);
            }

            ESP_LOGI(c_tag, "Connected");
        }

        static bool handle_send(tcp_client_t& client) {
            constexpr TickType_t wait_delay = 0;
            tcp_event_data_t event_data;

            // drain the event queue
            while (tcp_send_event_t::receive(event_data, wait_delay)) {
                uint8_t attempts = 0;

                while (true) {
                    if (attempts == CONFIG_TCP_TIMEOUT_ATTEMPTS) {
                        return false; // fully timed out
                    }

                    tcp_status_t status = event_data.send_to_client(client);

                    if (status == tcp_status_t::success) {
                        attempts = 0;
                        break; // move on the next event
                    }
                    else if (status == tcp_status_t::timeout) {
                        attempts++;
                        continue; // try again
                    }
                    else {
                        return false; // tcp error
                    }
                }
            }

            return true;
        }

        static bool handle_listen(tcp_client_t& client) {
            uint8_t attempts = 0;

            while (true) {
                if (attempts == CONFIG_TCP_TIMEOUT_ATTEMPTS) {
                    return false; // fully timed out
                }

                tcp_status_t status = client.listen_to_server();

                if (status == tcp_status_t::success) {
                    break;
                }
                else if (status == tcp_status_t::timeout) {
                    attempts++;
                    continue; // try again
                }
                else {
                    return false; // tcp error
                }
            }

            return true;
        }

    private:
        static void on_init_request(const common::esp_init_request_t& init_request) {
            ESP_LOGI(c_tag, "Received server response, sending ack...");

            tcp_send_event_t::send(tcp_event_data_t(
                common::esp_init_response_t {
                    .id = init_request.id
                }
            ));

            // store id somewhere
        }

        static void on_motor_control(const common::motor_control_t& motor_control) {
            motor_command_t::send(motor_control);
        }

    private:
        static constexpr const char* c_tag = "tcp_task";
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 8192;
    };        
}