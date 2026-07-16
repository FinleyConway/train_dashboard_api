#pragma once

#include <freertos/FreeRTOS.h>

#include "tasks/tcp_send_task.hpp"
#include "tasks/tcp_listen_task.hpp"
#include "networking/tcp_client.hpp"

namespace client {
    struct system_bus_t;

    class tcp_manager_task_t {
    public:
        tcp_manager_task_t(system_bus_t& system_bus);
        ~tcp_manager_task_t();

        tcp_manager_task_t(const tcp_manager_task_t&) = delete;
        tcp_manager_task_t& operator=(const tcp_manager_task_t&) = delete;

        tcp_manager_task_t(tcp_manager_task_t&&) noexcept = delete;
        tcp_manager_task_t& operator=(tcp_manager_task_t&&) noexcept = delete;

    public:
        void init();

        TaskHandle_t get_handle();

        tcp_client_t& get_client();

    private:
        static void run_wrapper(void* parameters);

        void run();

        void try_connect();

    private:
        static constexpr const char* c_tag = "tcp_task";

        tcp_client_t m_client;
        tcp_send_task_t m_tcp_read_task;
        tcp_listen_task_t m_tcp_listen_task;
        TaskHandle_t m_handle = nullptr;
    };
}