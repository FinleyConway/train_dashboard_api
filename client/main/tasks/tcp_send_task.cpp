#include "tasks/tcp_send_task.hpp"

#include <sdkconfig.h>

#include "system_bus.hpp"
#include "tasks/tcp_manager_task.hpp"
#include "utils/tcp_event_data.hpp"

namespace client {
    tcp_send_task_t::tcp_send_task_t(system_bus_t& bus, tcp_manager_task_t& manager) : 
        m_bus(bus), 
        m_manager(manager) 
    {
    }

    tcp_send_task_t::~tcp_send_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }
    }   

    void tcp_send_task_t::init() {
        xTaskCreate(
            run_wrapper,
            "tcp_send_task_t",
            CONFIG_TCP_TASK_STACK, 
            this,
            CONFIG_TCP_TASK_PRIORITY + 2,
            &m_handle
        );
    }

    void tcp_send_task_t::destroy() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
            m_handle = nullptr;
        }
    }

    void tcp_send_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<tcp_send_task_t*>(parameters);
        self->run();
    }

    void tcp_send_task_t::run() {
        while (true) {
            // block thread until we receive a send request
            tcp_event_data_t event_data;
            if (!m_bus.outgoing_messages.receive(event_data)) continue;

            uint8_t attempts = 0;

            while (true) {
                if (attempts == CONFIG_TCP_TIMEOUT_ATTEMPTS) {
                    xTaskNotifyGive(m_manager.get_handle());
                    vTaskSuspend(nullptr);
                    return;
                }

                tcp_status_t status = event_data.send_to_client(m_manager.get_client());

                if (status == tcp_status_t::success) {
                    attempts = 0;
                    break; // move on the next event
                }
                else if (status == tcp_status_t::timeout) {
                    attempts++;
                    continue; // try again
                }
                else {
                    xTaskNotifyGive(m_manager.get_handle());
                    vTaskSuspend(nullptr);
                    return;
                }
            }
        }

        vTaskDelete(nullptr);
        m_handle = nullptr;
    }
}