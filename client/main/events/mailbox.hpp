#pragma once

#include <freertos/FreeRTOS.h>
#include <esp_log.h>

namespace client {
    template<typename T>
    class mailbox_t {
    public:
        mailbox_t() {
            m_event = xQueueCreate(1, sizeof(T));
            if (m_event == nullptr) {
                ESP_LOGE("mailbox", "Can't create mailbox!");
                abort();
            }
        }

        ~mailbox_t() {
            vQueueDelete(m_event);
        }

        mailbox_t(const mailbox_t&) = delete;
        mailbox_t& operator=(const mailbox_t&) = delete;

        mailbox_t(mailbox_t&&) noexcept = default;
        mailbox_t& operator=(mailbox_t&&) noexcept = default;

        bool send(const T& destination) {
            return xQueueOverwrite(m_event, &destination) == pdPASS;
        }

        bool receive(T& data_ref, TickType_t timeout_tick = portMAX_DELAY) {
            return xQueueReceive(m_event, &data_ref, timeout_tick) == pdPASS;
        }

    private:
        QueueHandle_t m_event = nullptr;
    };
}