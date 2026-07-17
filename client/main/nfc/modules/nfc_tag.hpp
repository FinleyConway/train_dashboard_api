#pragma once

#include <span>
#include <array>
#include <cstdint>
#include <cstddef>

#include <pn532.h>

#include "nfc/parser/ndef_parser.hpp"

namespace client {
    class nfc_tag_t {
    public:
        nfc_tag_t() = default;

        explicit nfc_tag_t(NTAG2XX_MODEL ntag_model);

    private:
        // (NTAG2XX_NTAG216 * 4 bytes) - read_only bytes
        static constexpr size_t c_max_nfc_user_memory = 888; 
        static constexpr size_t c_max_uid_length = 10;

    public:
        using uid_t = std::array<uint8_t, c_max_uid_length>;
        using data_t = std::array<uint8_t, c_max_nfc_user_memory>;

        void set_model(NTAG2XX_MODEL ntag_model);

        void set_uid(uid_t&& uid, uint8_t uid_length);

        void set_payload(data_t&& payload, size_t read_bytes);

        NTAG2XX_MODEL get_model() const;

        std::span<const uint8_t> get_uid() const;

        ndef_record_view_t get_record() const;

        bool uid_equals(const uid_t& uid) const;

    private:
        uid_t m_uid{};
        uint8_t m_uid_length = 0;

        data_t m_data{};
        size_t m_read_bytes = 0;

        NTAG2XX_MODEL m_ntag_model = NTAG2XX_UNKNOWN;
    };
}