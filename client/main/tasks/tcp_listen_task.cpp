#include "tasks/tcp_listen_task.hpp"

#include "tasks/tcp_manager_task.hpp"

namespace client {
    tcp_listen_task_t::tcp_listen_task_t(system_bus_t& bus, tcp_manager_task_t& manager) : 
        m_bus(bus), 
        m_manager(manager) 
    {
    }

    tcp_listen_task_t::~tcp_listen_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }
    }

    void tcp_listen_task_t::init() {
        xTaskCreate(
            run_wrapper,
            "tcp_listen_task_t",
            8192, // TODO: ADD CONFIG
            this,
            5, // priority
            &m_handle
        );
    }

    void tcp_listen_task_t::destroy() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
            m_handle = nullptr;
        }
    }

    void tcp_listen_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<tcp_listen_task_t*>(parameters);
        self->run();
    }

    void tcp_listen_task_t::run() {
        while (true) {
            while (true) {
                tcp_status_t status = m_manager.get_client().listen_to_server();

                if (status == tcp_status_t::success) {
                    break;
                }
                else if (status == tcp_status_t::timeout) {
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