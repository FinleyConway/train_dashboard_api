#pragma once

#include <freertos/FreeRTOS.h>

#include "common/messages/motor_control.hpp"

namespace client {
    class motor_command_t {
    public:
        static void create() {
            if (s_event != nullptr) {
                // add log or assert

                return;
            }

            // creating a latest-value queue
            s_event = xQueueCreate(1, sizeof(common::motor_control_t));

            if (s_event == nullptr) {
                // add log
            }
        }

        static void send(const common::motor_control_t& motor_control) {
            if (s_event == nullptr) return; // add log or assert

            xQueueOverwrite(s_event, &motor_control);
        }

        static bool receive(common::motor_control_t& motor_control, TickType_t timeout_tick = portMAX_DELAY) {
            if (s_event == nullptr) return false; // add log or assert

            return xQueueReceive(s_event, &motor_control, timeout_tick) == pdPASS;
        }

    private:
        inline static QueueHandle_t s_event = nullptr;
    };
}