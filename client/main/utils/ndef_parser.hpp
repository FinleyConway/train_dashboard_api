#pragma once

#include <span>
#include <cstdint>
#include <string_view>

// https://freemindtronic.com/wp-content/uploads/2022/02/NFC-Data-Exchange-Format-NDEF.pdf
// https://github.com/haldean/ndef/blob/master/ndef.c
// https://tucker.the-twomeys.com/blog/posts/ndef-tlv/

namespace client {
    struct ndef_record_view_t {
        std::string_view type; 
        std::span<const uint8_t> payload;
    };

    class ndef_parser_t {
    public:
        static ndef_record_view_t parse(std::span<const uint8_t> data);

    private:
        static std::span<const uint8_t> try_get_record_view(const auto& data);

        static bool is_short_record(uint8_t header);

        static bool has_id_length(uint8_t header);

        static uint32_t get_payload_length(uint8_t header, const auto& data, size_t& index);

        static uint8_t get_id_length(uint8_t header, const auto& data, size_t& index);

    private:
        static constexpr const char* c_tag = "ndef_parser";
    };
}