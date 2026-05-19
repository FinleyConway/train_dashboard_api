#pragma once

#include "common/core/serialise.hpp"

namespace common {
    struct restart_esp_t { 
        static void serialise(std::span<uint8_t>& payload, const restart_esp_t& data) {}

        static restart_esp_t deserialise(std::span<uint8_t> payload) { return {}; }

        static constexpr size_t payload_size() { return sizeof(restart_esp_t); }
    };
}