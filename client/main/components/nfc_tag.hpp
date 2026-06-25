#pragma once

#include <span>
#include <array>
#include <cstdint>
#include <cstddef>

#include "utils/ndef_parser.hpp"

namespace client {
    class nfc_tag_t {
    private:
        // (NTAG2XX_NTAG216 * 4 bytes) - read_only bytes
        static constexpr size_t max_nfc_user_memory = 888; 
        static constexpr size_t max_uid_length = 10;

    public:
        using uid_t = std::array<uint8_t, max_uid_length>;
        using data_t = std::array<uint8_t, max_nfc_user_memory>;

        void set_uid(uid_t&& uid, uint8_t uid_length) {
            m_uid = std::move(uid);
            m_uid_length = uid_length;
        }

        void set_payload(data_t&& payload, size_t read_bytes) {
            m_data = std::move(payload);
            m_read_bytes = read_bytes;
        }

        std::span<const uint8_t> get_uid() const {
            return { m_uid.data(), m_uid_length };
        }

        ndef_record_view_t get_record() const {
            return ndef_parser_t::parse(
                std::span<const uint8_t>(m_data.data(), m_read_bytes)
            );
        }

    private:
        uid_t m_uid{};
        uint8_t m_uid_length = 0;

        data_t m_data{};
        size_t m_read_bytes = 0;
    };
}