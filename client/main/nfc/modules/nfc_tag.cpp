#include "nfc/modules/nfc_tag.hpp"

namespace client {
    nfc_tag_t::nfc_tag_t(NTAG2XX_MODEL ntag_model) : m_ntag_model(ntag_model) {
    }

    void nfc_tag_t::set_model(NTAG2XX_MODEL ntag_model) {
        m_ntag_model = ntag_model;
    }

    void nfc_tag_t::set_uid(nfc_tag_t::uid_t&& uid, uint8_t uid_length) {
        m_uid = std::move(uid);
        m_uid_length = uid_length;
    }

    void nfc_tag_t::set_payload(nfc_tag_t::data_t&& payload, size_t read_bytes) {
        m_data = std::move(payload);
        m_read_bytes = read_bytes;
    }

    NTAG2XX_MODEL nfc_tag_t::get_model() const {
        return m_ntag_model;
    }

    std::span<const uint8_t> nfc_tag_t::get_uid() const {
        return { m_uid.data(), m_uid_length };
    }

    ndef_record_view_t nfc_tag_t::get_record() const {
        return ndef_parser_t::parse({ m_data.data(), m_read_bytes });
    }

    bool nfc_tag_t::uid_equals(const uid_t& uid) const {
        return m_uid == uid;
    }
}