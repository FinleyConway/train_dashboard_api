#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace common {
    struct esp_init_request_t {
        common::esp_id_t id = 0;

        static void serialise(std::span<uint8_t>& payload, const esp_init_request_t& data) {
            serialise_t::write(payload, data.id);
        }

        static esp_init_request_t deserialise(std::span<uint8_t> payload) {
            esp_init_request_t result;

            result.id = serialise_t::read<uint16_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<uint16_t>();
        }
    };

    struct esp_init_response_t {
        common::esp_id_t id = 0;

        static void serialise(std::span<uint8_t>& payload, const esp_init_response_t& data) {
            serialise_t::write(payload, data.id);
        }

        static esp_init_response_t deserialise(std::span<uint8_t> payload) {
            esp_init_response_t result;

            result.id = serialise_t::read<uint16_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<uint16_t>();
        }
    };
}