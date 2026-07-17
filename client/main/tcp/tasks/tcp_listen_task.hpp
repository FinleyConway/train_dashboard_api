#pragma once

#include <freertos/FreeRTOS.h>

namespace client {
    struct system_bus_t;
    class tcp_manager_task_t;

    class tcp_listen_task_t {
    public:
        tcp_listen_task_t(system_bus_t& bus, tcp_manager_task_t& manager);
        ~tcp_listen_task_t(); 

        tcp_listen_task_t(const tcp_listen_task_t&) = delete;
        tcp_listen_task_t& operator=(const tcp_listen_task_t&) = delete;

        tcp_listen_task_t(tcp_listen_task_t&&) noexcept = delete;
        tcp_listen_task_t& operator=(tcp_listen_task_t&&) noexcept = delete;

        void init();
    
        void destroy();

    private:
        static void run_wrapper(void* parameters);

        void run();

    private:
        system_bus_t& m_bus;
        tcp_manager_task_t& m_manager;
        TaskHandle_t m_handle = nullptr;
    };
}