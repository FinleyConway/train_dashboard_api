#pragma once

#include <freertos/FreeRTOS.h>

namespace client {
    class tcp_client_t;

    class tcp_send_task_t {
    public:
        static void init(tcp_client_t& client);

        static void destroy();

    private:
        static void run(void* parameters);

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 8192;
    };
}