#include "tasks/nfc_task.hpp"

#include <sdkconfig.h>

#include "task_events/tcp/tcp_send_event.hpp"
#include "task_events/tcp/tcp_event_data.hpp"
#include "components/nfc_reader.hpp"
#include "components/nfc_tag.hpp"
#include "utils/rail_nfc.hpp"

#include "common/messages/rail_location.hpp"

namespace client {
    void nfc_task_t::init(common::esp_id_t id) {
        if (s_handle != nullptr) {
            vTaskDelete(s_handle);
        }

        xTaskCreate(
            run,
            "nfc_task_t",
            CONFIG_NFC_TASK_STACK,
            &id,
            CONFIG_NFC_TASK_PRIORITY,
            &s_handle
        );
    }

    void nfc_task_t::run(void* parameters) {
            common::esp_id_t train_id = *static_cast<common::esp_id_t*>(parameters);
            nfc_reader_t nfc_reader;

            ESP_ERROR_CHECK(nfc_reader.init(nfc_gpio_t {
                .miso  = static_cast<gpio_num_t>(CONFIG_NFC_MISO_GPIO),
                .mosi  = static_cast<gpio_num_t>(CONFIG_NFC_MOSI_GPIO),
                .sck   = static_cast<gpio_num_t>(CONFIG_NFC_SCK_GPIO),
                .cs    = static_cast<gpio_num_t>(CONFIG_NFC_CS_GPIO)
            }));

            while (true) {
                // stop thread and capture nfc infomation
                nfc_tag_t tag;
                nfc_read_state_t state = nfc_reader.wait_for_tag(tag);

                // have we atleast got information to work with?
                if (state != nfc_read_state_t::fail) {
                    ndef_record_view_t record = tag.get_record();
                    
                    if (record.type == "rail") {
                        // read the payload data and inform the server
                        auto rail = rail_nfc_t::deserialise(record.payload);

                        tcp_send_event_t::send(tcp_event_data_t(
                            common::rail_location_t {
                                .id = train_id,
                                .rail_id = rail.rail_id,
                                .type = rail.type
                            }
                        ));
                    }
                }
            }

            vTaskDelete(nullptr);
        }
}