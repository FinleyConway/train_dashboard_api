#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_err.h>

#include "common/messages/motor.hpp"

namespace client {
    class motor_command_t {
    public:
        static esp_err_t init() {
            if (s_event != nullptr) return ESP_OK;

            // creating a latest-value queue
            s_event = xQueueCreate(1, sizeof(common::motor_control_t));

            if (s_event == nullptr) {
                return ESP_ERR_NO_MEM;
            }

            return ESP_OK;
        }

        static bool send(const common::motor_control_t& motor_control) {
            configASSERT(s_event);

            return xQueueOverwrite(s_event, &motor_control);
        }

        static bool receive(common::motor_control_t& motor_control, TickType_t timeout_tick = portMAX_DELAY) {
           configASSERT(s_event);

            return xQueueReceive(s_event, &motor_control, timeout_tick) == pdPASS;
        }

    private:
        inline static QueueHandle_t s_event = nullptr;
    };
}