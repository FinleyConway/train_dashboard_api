#include "tasks/tcp/tcp_send_task.hpp"

#include "networking/tcp_client.hpp"
#include "task_events/tcp/tcp_event_data.hpp"
#include "task_events/tcp/tcp_send_event.hpp"
#include "tasks/tcp/tcp_manager_task.hpp"

namespace client {
    void tcp_send_task_t::init(tcp_client_t& client) {
        xTaskCreate(
            run,
            "tcp_send_task_t",
            s_stack_size,
            &client,
            5, // priority
            &s_handle
        );
    }

    void tcp_send_task_t::destroy() {
        if (s_handle != nullptr) {
            vTaskDelete(s_handle);
            s_handle = nullptr;
        }
    }

    void tcp_send_task_t::run(void* parameters) {
        auto& client = *static_cast<tcp_client_t*>(parameters);

        while (true) {
            // block thread until we receive a send request
            tcp_event_data_t event_data;
            if (!tcp_send_event_t::receive(event_data)) continue;

            uint8_t attempts = 0;

            while (true) {
                if (attempts == CONFIG_TCP_TIMEOUT_ATTEMPTS) {
                    xTaskNotifyGive(tcp_manager_task_t::get_handle());
                    vTaskSuspend(nullptr);
                    return;
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
                    xTaskNotifyGive(tcp_manager_task_t::get_handle());
                    vTaskSuspend(nullptr);
                    return;
                }
            }
        }

        s_handle = nullptr;
        vTaskDelete(nullptr);
    }
}