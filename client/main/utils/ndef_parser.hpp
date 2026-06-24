#pragma once

#include <span>
#include <cstdint>
#include <string_view>

#include "components/nfc_reader.hpp"

// https://freemindtronic.com/wp-content/uploads/2022/02/NFC-Data-Exchange-Format-NDEF.pdf
// https://github.com/haldean/ndef/blob/master/ndef.c

namespace client {
    struct ndef_record_view_t { // TODO: maybe store this inside the nfc_tag and keep nfc_tag alive until the next scan nfc_tag is only short lived
        std::string_view type; 
        std::span<const uint8_t> payload;
    };

    class ndef_parser_t {
    public:
        ndef_record_view_t parse(client::nfc_tag_t& tag) {
            const auto data = std::span<const uint8_t>(tag.data.data(), tag.data_length);

            // check if i can read the vital info
            // has the reader captured enough data?
            if (data.size() < 3) return {}; 

            size_t index = 0;

            // read the header infomation about the record layout
            const uint8_t header = data[index++];
            const bool has_id_length = has_id_length(header);

            // read the type length
            const uint8_t type_length = data[index++];

            // read the payload length
            const uint32_t payload_length = get_payload_length(header, data, index);

            // read the id length and advance to the next byte (dont need it)
            index += get_id_length(header, data, index);

            // check if the reader captured the payload
            if (index + type_length + payload_length > data.size()) return {};

            // not sure about turning uint8_t to a char* but it should be fine?
            std::string_view type(
                reinterpret_cast<const char*>(data.data() + index), 
                type_length
            );

            index += type_length;

            std::span<const uint8_t> payload(
                data.data() + index,
                payload_length
            );

            return { type, payload };
        }

    private:
        bool is_short_record(uint8_t header) {
            return (header & 0x10) != 0;
        }

        bool has_id_length(uint8_t header) {
            return (header & 0x08) != 0;
        }

        uint32_t get_payload_length(uint8_t header, const auto& data, size_t& index) {
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
                return 0;
            }

            uint32_t payload_length = 0;

            // NDEF stores the long payload length as big-endian.
            for (int i = 0; i < 4; ++i) {
                payload_length = (payload_length << 8) | data[index++];
            }

            return payload_length;
        }

        uint8_t get_id_length(uint8_t header, const auto& data, size_t& index) {
            const bool has_length = has_id_length(header);
            uint8_t id_length = 0;

            if (has_length) {
                // has the reader captured enough data?
                if (index >= data.size()) {
                    return 0;
                }

                id_length = data[index++];
            }

            return id_length;
        }
    };
}