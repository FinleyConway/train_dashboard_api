#pragma once

#include <freertos/FreeRTOS.h>
#include <sdkconfig.h>
#include <esp_err.h>

#include "task_events/tcp/tcp_event_data.hpp"

namespace client {
    class tcp_send_event_t {
    public:
        static bool send(const tcp_event_data_t& data, TickType_t block_tick = pdMS_TO_TICKS(10)) {
            ESP_ERROR_CHECK(try_create());

            return xQueueSend(s_event, &data, block_tick) == pdPASS;
        }

        static bool receive(tcp_event_data_t& data_ref, TickType_t timeout_tick = portMAX_DELAY) {
            if (s_event == nullptr) return false;

            return xQueueReceive(s_event, &data_ref, timeout_tick) == pdPASS;
        }

    private:
        static esp_err_t try_create() {
            if (s_event != nullptr) return ESP_OK;

            s_event = xQueueCreate(CONFIG_TCP_EVENT_QUEUE_SIZE, sizeof(tcp_event_data_t));

            if (s_event == nullptr) {
                return ESP_ERR_NO_MEM;
            }

            return ESP_OK;
        }

    private:
        inline static QueueHandle_t s_event = nullptr;
    };
}