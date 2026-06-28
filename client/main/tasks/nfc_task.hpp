#pragma once

#include <freertos/FreeRTOS.h>

#include "components/nfc_reader.hpp"
#include "components/nfc_tag.hpp"
#include "utils/rail_location.hpp"

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
            nfc_reader_t nfc_reader;

            ESP_ERROR_CHECK(nfc_reader.init(nfc_gpio_t {
                .misco = GPIO_NUM_19,
                .mosi  = GPIO_NUM_18,
                .sck   = GPIO_NUM_21,
                .cs    = GPIO_NUM_5
            }));

            ESP_LOGI("nfc", "Waiting for an ISO14443A Card ...");

            while (true) {
                nfc_tag_t tag;
                nfc_read_state state = nfc_reader.read_tag(tag);

                if (state != nfc_read_state::fail) {
                    ndef_record_view_t record = tag.get_record();
                    
                    if (record.type == "rail") {
                        ESP_LOG_BUFFER_HEXDUMP("nfc", record.payload.data(), record.payload.size(), ESP_LOG_INFO);

                        auto rail = rail_location_t::deserialise(record.payload);

                        ESP_LOGI("nfc", "Rail: (id: %llu, type: %d)", rail.id, rail.type);
                    }
                }
            }

            vTaskDelete(nullptr);
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 2048 * 2;
    };
} 