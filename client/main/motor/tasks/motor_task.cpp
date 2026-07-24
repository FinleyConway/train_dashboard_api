#include "motor/tasks/motor_task.hpp"

#include <cmath>
#include <algorithm>

#include <sdkconfig.h>

#include "app/system_bus.hpp"

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
            return;
        }

        xTaskCreate(
            run_wrapper,
            "motor_task_t",
            CONFIG_MOTOR_TASK_STACK,
            this,
            CONFIG_MOTOR_TASK_PRIORITY,
            &m_handle
        );
    }

    void motor_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<motor_task_t*>(parameters);
        self->run();
    }

    void motor_task_t::run() {
        m_motor.init(motor_gpio_t {
            .pwm_channel = static_cast<ledc_channel_t>(CONFIG_MOTOR_PWM_CHANNEL),
            .pwm         = static_cast<gpio_num_t>(CONFIG_MOTOR_PWM_GPIO),
            .ain_a       = static_cast<gpio_num_t>(CONFIG_MOTOR_AIN_A_GPIO),
            .ain_b       = static_cast<gpio_num_t>(CONFIG_MOTOR_AIN_B_GPIO),
            .standby     = static_cast<gpio_num_t>(CONFIG_MOTOR_STANDBY_GPIO)
        });

        while (true) {
            // block thread and wait for motor_control commands
            common::motor_control_t motor_control;

            if (m_bus.motor_command.receive(motor_control)) { 
                // do motor stuff
                m_motor.set_active_state(motor_control.is_active);
                m_motor.set_motor_direction(client::motor_direction_t::clockwise); // need to control this via tcp

                send_motor_status();

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

        const TickType_t log_interval = pdMS_TO_TICKS(100);
        TickType_t last_log_tick = start_tick;

        while (true) {
            const TickType_t now = xTaskGetTickCount();
            const float elapsed_ms = (now - start_tick) * portTICK_PERIOD_MS;
            const float t = elapsed_ms / static_cast<float>(motor_control.ramp_time_ms);

            if (t >= 1.0f) break;

            const float eased = ease(std::clamp(t, 0.0f, 1.0f));
            const float value = std::lerp(start, end, eased);

            m_motor.set_motor_duty(static_cast<uint32_t>(std::round(value)));

            if ((now - last_log_tick) >= log_interval) {
                send_motor_status();
                last_log_tick = now;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        m_motor.set_motor_duty(motor_control.target_duty);

        send_motor_status();
    }

    void motor_task_t::send_motor_status() {
        m_bus.outgoing_messages.send(tcp_event_data_t {
            common::motor_status_t {
                .id = m_bus.train_id,
                .current_duty = m_motor.get_current_duty(),
                .is_active = m_motor.is_active()
            }
        });
    }

    float motor_task_t::ease(float x) {
        return x * x * x;
    }
}