#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace common {
    struct battery_status_t {
        common::esp_id_t id;
        float percentage_level;

        static void serialise(std::span<uint8_t>& payload, const battery_status_t& data) {
            serialise_t::write(payload, data.id);
            serialise_t::write(payload, data.percentage_level);
        }

        static battery_status_t deserialise(std::span<const uint8_t> payload) {
            battery_status_t result;

            result.id = serialise_t::read<common::esp_id_t>(payload); 
            result.percentage_level = serialise_t::read<float>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                common::esp_id_t,
                float
            >();
        }
    };
}