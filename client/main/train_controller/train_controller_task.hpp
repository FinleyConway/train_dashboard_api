#pragma once

#include <freertos/FreeRTOS.h>

namespace client {
    struct system_bus_t;

    class train_controller_task_t {
    public:
        train_controller_task_t(system_bus_t& bus);
        ~train_controller_task_t();

        train_controller_task_t(const train_controller_task_t&) = delete;
        train_controller_task_t& operator=(const train_controller_task_t&) = delete;

        train_controller_task_t(train_controller_task_t&&) noexcept = delete;
        train_controller_task_t& operator=(train_controller_task_t&&) noexcept = delete;

    public:
        void init();

    private:
        static void run_wrapper(void* parameters);

        void run();

    private:
        static constexpr const char* c_tag = "train_controller";

        system_bus_t& m_bus;
        TaskHandle_t m_handle = nullptr;
    };
}