#include "utils/ndef_parser.hpp"

#include <esp_log.h>

namespace client {
    ndef_record_view_t ndef_parser_t::parse(std::span<const uint8_t> data) {
        std::span<const uint8_t> record_view = try_get_record_view(data);
        if (record_view.empty()) return {};

        uint8_t index = 0;

        // read the header infomation about the record layout
        const uint8_t header = record_view[index++];

        // read the type length
        const uint8_t type_length = record_view[index++];
        ESP_LOGI(c_tag, "Incoming type length: %u", type_length);

        // read the payload length
        const uint32_t payload_length = get_payload_length(header, record_view, index);
        ESP_LOGI(c_tag, "Incoming payload length: %lu", payload_length);

        // read the id length
        const uint8_t id_length = get_id_length(header, record_view, index);
        ESP_LOGI(c_tag, "Incoming id length: %u", id_length);

        // not sure about turning uint8_t to a char* but it should be fine?
        std::string_view type(
            reinterpret_cast<const char*>(record_view.data() + index), 
            type_length
        );
        index += type_length;

        // i dont care about the id
        index += id_length;

        std::span<const uint8_t> payload(
            record_view.data() + index,
            payload_length
        );

        return { type, payload };
    }

    std::span<const uint8_t> ndef_parser_t::try_get_record_view(const auto& data) {
        uint8_t index = 0;

        // are the next leading bytes a ndef message
        if (data[index++] != 0x03) {
            ESP_LOGE(c_tag, "Given data is not a ndef message");

            return {};
        }

        uint16_t record_length = 0;
        const uint8_t len_byte = data[index++];

        // is the record larger then 255?
        if (len_byte == 0xFF) {
            if (index + 2 > data.size()) {
                ESP_LOGW(c_tag, "Not enough captured data to parse ndef record length");

                return {};
            }

            // NDEF stores the long record length as big-endian.
            record_length = (data[index] << 8) | (data[index + 1]); // from left to right

            index += 2;
        } 
        else {
            record_length = len_byte;
        }

        if (index + record_length > data.size()) {
            ESP_LOGW(c_tag, "Not enough captured data to parse ndef record");

            return {};
        }

        return { data.data() + index, record_length };
    }

    bool ndef_parser_t::is_short_record(uint8_t header) {
        return (header & 0x10) != 0;
    }

    bool ndef_parser_t::has_id_length(uint8_t header) {
        return (header & 0x08) != 0;
    }

    uint32_t ndef_parser_t::get_payload_length(uint8_t header, const auto& data, uint8_t& index) {
        const bool short_record = is_short_record(header);

        ESP_LOGI(c_tag, "Is short record: %d", short_record);

        if (short_record) {
            // has the reader captured enough data?
            if (index >= data.size()) {
                return 0; 
            }

            return data[index++];
        }

        uint32_t payload_length = 0;

        // NDEF stores the long payload length as big-endian.
        for (uint8_t i = 0; i < sizeof(uint32_t); i++) {
            payload_length = (payload_length << 8) | data[index++]; // from left to right
        }

        return payload_length;
    }

    uint8_t ndef_parser_t::get_id_length(uint8_t header, const auto& data, uint8_t& index) {
        const bool has_length = has_id_length(header);

        ESP_LOGI(c_tag, "Has id length: %d", has_length);

        if (has_length) {
            return data[index++];
        }

        return 0;
    }
}