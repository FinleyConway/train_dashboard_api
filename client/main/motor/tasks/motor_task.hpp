#pragma once

#include <freertos/FreeRTOS.h>

#include "motor/modules/motor.hpp"
#include "common/messages/motor.hpp"

namespace client {
    struct system_bus_t;

    class motor_task_t {
    public:
        explicit motor_task_t(system_bus_t& bus);
        ~motor_task_t();

        motor_task_t(const motor_task_t&) = delete;
        motor_task_t& operator=(const motor_task_t&) = delete;

        motor_task_t(motor_task_t&&) noexcept = delete;
        motor_task_t& operator=(motor_task_t&&) noexcept = delete;

    public:
        void init();

    private:
        static void run_wrapper(void* parameters);

        void run();

        void adjust_motor(const common::motor_control_t& motor_control);

        static float ease(float x);

    private:
        motor_t m_motor;
        system_bus_t& m_bus;
        TaskHandle_t m_handle = nullptr;
    };
} 