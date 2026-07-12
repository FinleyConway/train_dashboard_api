#pragma once

#include <freertos/FreeRTOS.h>
#include <sdkconfig.h>

#include "components/nfc_reader.hpp"
#include "components/nfc_tag.hpp"

#include "common/messages/rail_location.hpp"

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
                .miso  = static_cast<gpio_num_t>(CONFIG_NFC_MISO_GPIO),
                .mosi  = static_cast<gpio_num_t>(CONFIG_NFC_MOSI_GPIO),
                .sck   = static_cast<gpio_num_t>(CONFIG_NFC_SCK_GPIO),
                .cs    = static_cast<gpio_num_t>(CONFIG_NFC_CS_GPIO)
            }));

            while (true) {
                nfc_tag_t tag;
                nfc_read_state state = nfc_reader.read_tag(tag);

                if (state != nfc_read_state::fail) {
                    ndef_record_view_t record = tag.get_record();
                    
                    if (record.type == "rail") {
                        ESP_LOG_BUFFER_HEXDUMP(c_tag, record.payload.data(), record.payload.size(), ESP_LOG_INFO);

                        auto rail = common::rail_location_t::deserialise(record.payload);

                        ESP_LOGI(c_tag, "Rail: (id: %llu, type: %d)", rail.id, rail.type);

                        tcp_send_event_t::send(tcp_event_data_t(rail));
                    }
                }
            }

            vTaskDelete(nullptr);
        }

    private:
        static constexpr const char* c_tag = "nfc-task";
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 2048 * 2;
    };
} 