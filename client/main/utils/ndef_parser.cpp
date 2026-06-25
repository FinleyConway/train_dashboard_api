#include "utils/ndef_parser.hpp"

#include <esp_log.h>

namespace client {
    ndef_record_view_t ndef_parser_t::parse(std::span<const uint8_t> data) {
        // check if i can read the vital info
        // has the reader captured enough data?
        if (data.size() < 3) {
            ESP_LOGW("ndef", "Not enough to parse metadata");
            
            return {}; 
        } 

        size_t index = 0;

        // read the header infomation about the record layout
        const uint8_t header = data[index++];

        // read the type length
        const uint8_t type_length = data[index++];

        // read the payload length
        const uint32_t payload_length = get_payload_length(header, data, index);

        // read the id length and advance to the next byte (dont need it)
        index += get_id_length(header, data, index);

        // not sure about turning uint8_t to a char* but it should be fine?
        std::string_view type(
            reinterpret_cast<const char*>(data.data() + index), 
            type_length
        );

        index += type_length;

        if (index + payload_length > data.size()) {
            ESP_LOGW("ndef", "Not enough to parse payload");

            return {};
        }

        std::span<const uint8_t> payload(
            data.data() + index,
            payload_length
        );

        return { type, payload };
    }

    bool ndef_parser_t::is_short_record(uint8_t header) {
        return (header & 0x10) != 0;
    }

    bool ndef_parser_t::has_id_length(uint8_t header) {
        return (header & 0x08) != 0;
    }

    uint32_t ndef_parser_t::get_payload_length(uint8_t header, const auto& data, size_t& index) {
        const bool short_record = is_short_record(header);

        if (short_record) {
            // has the reader captured enough data?
            if (index >= data.size()) {
                return 0;
            }

            return data[index++];
        }

        // has the reader captured enough data?
        if (index + 4 > data.size()) {
            ESP_LOGW("ndef", "Not enough to parse payload length");

            return 0;
        }

        uint32_t payload_length = 0;

        // NDEF stores the long payload length as big-endian.
        for (uint8_t i = 0; i < 4; i++) {
            payload_length = (payload_length << 8) | data[index++];
        }

        return payload_length;
    }

    uint8_t ndef_parser_t::get_id_length(uint8_t header, const auto& data, size_t& index) {
        const bool has_length = has_id_length(header);
        uint8_t id_length = 0;

        if (has_length) {
            // has the reader captured enough data?
            if (index >= data.size()) {
                ESP_LOGW("ndef", "Not enough to parse id length");

                return 0;
            }

            id_length = data[index++];
        }

        return id_length;
    }
}