#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace common {
    struct rail_destination_t {
        rail_id_t id;

        static void serialise(std::span<uint8_t>& payload, const rail_destination_t& data) {
            serialise_t::write(payload, data.id);
        }

        static rail_destination_t deserialise(std::span<const uint8_t> payload) {
            rail_destination_t result;

            result.id = serialise_t::read<rail_id_t>(payload);

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                rail_id_t
            >();
        }
    };
}