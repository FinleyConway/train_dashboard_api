#include "tasks/tcp/tcp_manager_task.hpp"

#include <sdkconfig.h>
#include <esp_log.h>

#include "networking/tcp_client.hpp"

#include "tasks/tcp/tcp_send_task.hpp"
#include "tasks/tcp/tcp_listen_task.hpp"

#include "task_events/motor_command.hpp"
#include "task_events/station_command.hpp"
#include "task_events/tcp/tcp_event_data.hpp"
#include "task_events/tcp/tcp_send_event.hpp"

namespace client {
    void tcp_manager_task_t::init(connection_callback_t&& callback) {
        xTaskCreate(
            run,
            "tcp_task_t",
            s_stack_size,
            nullptr,
            3, // priority
            &s_handle
        );

        s_connection_callback = std::move(callback);
    }

    TaskHandle_t tcp_manager_task_t::get_handle() {
        return s_handle;
    }

    void tcp_manager_task_t::run(void* parameters) {
        tcp_client_t client;

        register_messages(client);

        while (true) {
            // connect to the server
            try_connect(client);

            // create tcp i/o tasks
            tcp_send_task_t::init(client);
            tcp_listen_task_t::init(client);

            // listen for if any i/o tasks failures
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ESP_LOGI(c_tag, "Connection lost");

            tcp_send_task_t::destroy();
            tcp_listen_task_t::destroy();

            client.disconnect();
        }

        s_handle = nullptr;
        vTaskDelete(nullptr);
    }

    void tcp_manager_task_t::register_messages(tcp_client_t& client) {
        client.register_receieve_callback<common::esp_init_request_t, &on_init_request>();
        client.register_receieve_callback<common::motor_control_t, &on_motor_control>();
        client.register_receieve_callback<common::rail_destination_t, &on_station_add>();
    }

    void tcp_manager_task_t::try_connect(tcp_client_t& client) {
        constexpr TickType_t retry_delay = pdMS_TO_TICKS(5000);
        
        ESP_LOGI(c_tag, "Connecting...");

        while (client.try_connect(CONFIG_TCP_TIMEOUT) != tcp_status_t::success) {
            vTaskDelay(retry_delay);
        }

        ESP_LOGI(c_tag, "Connected");
    }

    void tcp_manager_task_t::on_init_request(const common::esp_init_request_t& init_request) {
        ESP_LOGI(c_tag, "Received server response, sending ack...");

        tcp_send_event_t::send(tcp_event_data_t(
            common::esp_init_response_t {
                .id = init_request.id
            }
        ));

        if (s_connection_callback != nullptr) {
            s_connection_callback(init_request.id);
        }
    }

    void tcp_manager_task_t::on_motor_control(const common::motor_control_t& motor_control) {
        motor_command_t::send(motor_control);
    }

    void tcp_manager_task_t::on_station_add(const common::rail_destination_t& rail_destination) {
        station_command_t::send(rail_destination.id);
    }
}
