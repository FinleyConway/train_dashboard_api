#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_err.h>

#include "common/api/types.hpp"

namespace client {
    class station_command_t {
    public:
        static esp_err_t init() {
            if (s_event != nullptr) return ESP_OK;

            // creating a latest-value queue
            s_event = xQueueCreate(1, sizeof(common::rail_id_t));

            if (s_event == nullptr) {
                return ESP_ERR_NO_MEM;
            }

            return ESP_OK;
        }

        static bool send(common::rail_id_t destination) {
            configASSERT(s_event);

            return xQueueOverwrite(s_event, &destination) == pdPASS;
        }

        static bool receive(common::rail_id_t& data_ref, TickType_t timeout_tick = portMAX_DELAY) {
            configASSERT(s_event);

            return xQueueReceive(s_event, &data_ref, timeout_tick) == pdPASS;
        }
        
    private:
        inline static QueueHandle_t s_event = nullptr;
    };
}