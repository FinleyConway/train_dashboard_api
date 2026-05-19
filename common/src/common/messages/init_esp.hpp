#pragma once

#include "common/core/serialise.hpp"

namespace common {
    struct init_esp_t {
        uint16_t id = 0;

        static void serialise(std::span<uint8_t>& payload, const init_esp_t& data) {
            serialise_t::write(payload, data.id);
        }

        static init_esp_t deserialise(std::span<uint8_t> payload) {
            init_esp_t result;

            result.id = serialise_t::read<uint16_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return sizeof(
                uint16_t
            );
        }
    };
}