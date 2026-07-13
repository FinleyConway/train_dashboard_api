#include "tasks/tcp/tcp_listen_task.hpp"

#include "networking/tcp_client.hpp"
#include "tasks/tcp/tcp_manager_task.hpp"

namespace client {
    void tcp_listen_task_t::init(tcp_client_t& client) {
        xTaskCreate(
            run,
            "tcp_listen_task_t",
            s_stack_size,
            &client,
            5, // priority
            &s_handle
        );
    }

    void tcp_listen_task_t::destroy() {
        if (s_handle != nullptr) {
            vTaskDelete(s_handle);
            s_handle = nullptr;
        }
    }

    void tcp_listen_task_t::run(void* parameters) {
        auto& client = *static_cast<tcp_client_t*>(parameters);

        while (true) {
            while (true) {
                tcp_status_t status = client.listen_to_server();

                if (status == tcp_status_t::success) {
                    break;
                }
                else if (status == tcp_status_t::timeout) {
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