#include "tasks/nfc_task.hpp"

#include <sdkconfig.h>

#include "system_bus.hpp"
#include "utils/rail_nfc.hpp"
#include "components/nfc_tag.hpp"
#include "common/messages/rail_location.hpp"

namespace client {
    nfc_task_t::nfc_task_t(system_bus_t& bus) : m_bus(bus) {
    }
    
    nfc_task_t::~nfc_task_t() {
        if (m_handle != nullptr) {
            vTaskDelete(m_handle);
        }
    }

    void nfc_task_t::init() {
        xTaskCreate(
            run_wrapper,
            "nfc_task_t",
            CONFIG_NFC_TASK_STACK,
            this,
            CONFIG_NFC_TASK_PRIORITY,
            &m_handle
        );
    }

    void nfc_task_t::run_wrapper(void* parameters) {
        auto* self = static_cast<nfc_task_t*>(parameters);
        self->run();
    }

    void nfc_task_t::run() {
        ESP_ERROR_CHECK(m_nfc_reader.init(nfc_gpio_t {
            .miso  = static_cast<gpio_num_t>(CONFIG_NFC_MISO_GPIO),
            .mosi  = static_cast<gpio_num_t>(CONFIG_NFC_MOSI_GPIO),
            .sck   = static_cast<gpio_num_t>(CONFIG_NFC_SCK_GPIO),
            .cs    = static_cast<gpio_num_t>(CONFIG_NFC_CS_GPIO)
        }));

        nfc_tag_t::uid_t previous_tag{};

        while (true) {
            // stop thread and capture nfc infomation
            nfc_tag_t tag;
            nfc_read_state_t state = m_nfc_reader.wait_for_tag(tag);
            
            // don't bother doing the next instructions we haven't scanned anything useful
            if (state == nfc_read_state_t::fail) continue;

            // have we scanned the same tag?
            if (tag.uid_equals(previous_tag)) continue;

            // remember this tag 
            std::copy(tag.get_uid().begin(), tag.get_uid().end(), previous_tag.begin());

            process_tag(tag);
        }

        vTaskDelete(nullptr);
        m_handle = nullptr;
    }

    void nfc_task_t::process_tag(const nfc_tag_t& tag) {
        ndef_record_view_t record = tag.get_record();
                
        if (record.type == "rail") {
            // read the payload data
            const auto rail = rail_nfc_t::deserialise(record.payload);

            // notify server
            m_bus.outgoing_messages.send(tcp_event_data_t(
                common::rail_location_t {
                    .id = m_bus.train_id,
                    .rail_id = rail.rail_id,
                    .type = rail.type
                }
            ));

            // notify train controller
            m_bus.current_rail.send(rail.rail_id);
        }
    }
}