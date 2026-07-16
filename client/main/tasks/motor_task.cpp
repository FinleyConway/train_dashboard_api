#include "tasks/motor_task.hpp"

#include <cmath>
#include <algorithm>

#include "system_bus.hpp"

namespace client {
    motor_task_t::motor_task_t(system_bus_t& bus) : m_bus(bus) {
    }
    
    motor_task_t::~motor_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }
    }

    void motor_task_t::init() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }

        xTaskCreate(
            run_wrapper,
            "motor_task_t",
            2048, // TODO: Add config
            this,
            3, // TODO: Add config
            &m_handle
        );
    }

    void motor_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<motor_task_t*>(parameters);
        self->run();
    }

    void motor_task_t::run() {
        m_motor.init(motor_gpio_t {
            .pwm_channel = LEDC_CHANNEL_0, // TODO: Add config
            .pwm = GPIO_NUM_33,
            .ain_a = GPIO_NUM_26,
            .ain_b = GPIO_NUM_32,
            .standby = GPIO_NUM_25
        });

        while (true) {
            // block thread and wait for motor_control commands
            common::motor_control_t motor_control;

            if (m_bus.motor_command.receive(motor_control)) { 
                // do motor stuff
                m_motor.set_active_state(motor_control.is_active);
                m_motor.set_motor_direction(client::motor_direction_t::clockwise); // need to control this via tcp

                if (motor_control.is_active) {
                    adjust_motor(motor_control);
                }
            }
        }

        vTaskDelete(nullptr);
        m_handle = nullptr;
    }

    void motor_task_t::adjust_motor(const common::motor_control_t& motor_control) {
        float start = static_cast<float>(motor_control.starting_duty);
        float end = static_cast<float>(motor_control.target_duty);

        const TickType_t start_tick = xTaskGetTickCount();

        while (true) {
            TickType_t now = xTaskGetTickCount();
            float elapsed_ms = (now - start_tick) * portTICK_PERIOD_MS;
            float t = elapsed_ms / static_cast<float>(motor_control.ramp_time_ms);

            if (t >= 1.0f) break;

            float eased = ease(std::clamp(t, 0.0f, 1.0f));
            float value = std::lerp(start, end, eased);

            m_motor.set_motor_duty(static_cast<uint32_t>(std::round(value)));

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        m_motor.set_motor_duty(motor_control.target_duty);
    }

    float motor_task_t::ease(float x) {
        return x * x * x;
    }
}