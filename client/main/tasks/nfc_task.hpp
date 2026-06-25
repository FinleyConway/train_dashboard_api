#pragma once

#include <freertos/FreeRTOS.h>

#include "components/nfc_reader.hpp"
#include "components/nfc_tag.hpp"

namespace client {
    class nfc_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            xTaskCreate(
                run,
                "nfc_task_t",
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
            client::nfc_reader_t nfc_reader;

            ESP_ERROR_CHECK(nfc_reader.init(client::nfc_gpio_t {
                .misco = GPIO_NUM_19,
                .mosi  = GPIO_NUM_18,
                .sck   = GPIO_NUM_21,
                .cs    = GPIO_NUM_5
            }));

            ESP_LOGI("nfc", "Waiting for an ISO14443A Card ...");

            while (true) {
                client::nfc_tag_t tag;
                nfc_reader.read_tag(tag);

                client::ndef_record_view_t record = tag.get_record();

                ESP_LOGI(
                    "nfc",
                    "Tag type: %.*s",
                    (int)record.type.size(),
                    record.type.data()
                );
                ESP_LOG_BUFFER_HEXDUMP("nfc", record.payload.data(), record.payload.size(), ESP_LOG_INFO);
            }

            vTaskDelete(nullptr);
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 2048 * 2;
    };
} 