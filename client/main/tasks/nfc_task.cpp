#include "tasks/nfc_task.hpp"

#include <sdkconfig.h>

#include "task_events/tcp/tcp_send_event.hpp"
#include "task_events/tcp/tcp_event_data.hpp"
#include "task_events/rail_command.hpp"
#include "components/nfc_reader.hpp"
#include "components/nfc_tag.hpp"
#include "utils/rail_nfc.hpp"

#include "common/messages/rail_location.hpp"
#include "nfc_task.hpp"

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

        nfc_tag_t::uid_t previous_tag;

        while (true) {
            // stop thread and capture nfc infomation
            nfc_tag_t tag;
            nfc_read_state_t state = nfc_reader.wait_for_tag(tag);
            
            // don't bother doing the next instructions we haven't scanned anything useful
            if (state == nfc_read_state_t::fail) continue;

            // have we scanned the same tag?
            if (tag.uid_equals(previous_tag)) continue;

            // remember this tag 
            std::copy(tag.get_uid().begin(), tag.get_uid().end(), previous_tag.begin());

            process_tag(train_id, tag);
        }

        vTaskDelete(nullptr);
    }

    void nfc_task_t::process_tag(common::esp_id_t id, const nfc_tag_t& tag) {
        ndef_record_view_t record = tag.get_record();
                
        if (record.type == "rail") {
            // read the payload data
            const auto rail = rail_nfc_t::deserialise(record.payload);

            // notify server
            tcp_send_event_t::send(tcp_event_data_t(
                common::rail_location_t {
                    .id = id,
                    .rail_id = rail.rail_id,
                    .type = rail.type
                }
            ));

            // notify train controller
            rail_command_t::send(rail.rail_id);
        }
    }
}