#pragma once

#include <cmath>
#include <algorithm>

#include <freertos/FreeRTOS.h>

#include "common/messages/motor.hpp"
#include "task_events/motor_command.hpp"
#include "components/motor.hpp"

namespace client {
    class motor_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            if (s_handle != nullptr) {
                vTaskDelete(s_handle);
            }

            xTaskCreate(
                run,
                "motor_task_t",
                s_stack_size,
                nullptr,
                tskIDLE_PRIORITY + priority_offset,
                &s_handle
            );
        }
        
        static TaskHandle_t get_handle() {
            return s_handle;
        }

    private:
        static void run(void* parameters) {
            motor_t motor;
            
            motor.init(motor_gpio_t {
                .pwm_channel = LEDC_CHANNEL_0,
                .pwm = GPIO_NUM_33,
                .ain_a = GPIO_NUM_26,
                .ain_b = GPIO_NUM_32,
                .standby = GPIO_NUM_25
            });

            while (true) {
                // block thread and wait for motor_control commands
                common::motor_control_t motor_control;

                if (motor_command_t::receive(motor_control)) { 
                    // do motor stuff
                    motor.set_active_state(motor_control.is_active);
                    motor.set_motor_direction(client::motor_direction_t::clockwise); // need to control this via tcp

                    if (motor_control.is_active) {
                        adjust_motor(motor, motor_control);
                    }
                }
            }

            vTaskDelete(nullptr);
        }

        static void adjust_motor(motor_t& motor, const common::motor_control_t& motor_control) {
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

                motor.set_motor_duty(static_cast<uint32_t>(std::round(value)));

                vTaskDelay(pdMS_TO_TICKS(10));
            }

            motor.set_motor_duty(motor_control.target_duty);
        }

        static float ease(float x) {
            return x * x * x;
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 2048;
    };
} 