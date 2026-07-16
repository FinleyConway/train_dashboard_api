#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_log.h>

namespace client {
    template<typename T>
    class event_queue_t {
    public:
        event_queue_t(size_t queue_size) {
            m_event = xQueueCreate(queue_size, sizeof(T));
            if (m_event == nullptr) {
                ESP_LOGE("event_queue", "Can't create event queue!");
                abort();
            }
        }

        ~event_queue_t() {
            vQueueDelete(m_event);
        }

        event_queue_t(const event_queue_t&) = delete;
        event_queue_t& operator=(const event_queue_t&) = delete;

        event_queue_t(event_queue_t&&) noexcept = default;
        event_queue_t& operator=(event_queue_t&&) noexcept = default;

        bool send(const T& data, TickType_t block_tick = 0) {
            return xQueueSend(m_event, &data, block_tick) == pdPASS;
        }

        bool receive(T& data_ref, TickType_t timeout_tick = portMAX_DELAY) {
            return xQueueReceive(m_event, &data_ref, timeout_tick) == pdPASS;
        }

    private:
        QueueHandle_t m_event = nullptr;
    };
}