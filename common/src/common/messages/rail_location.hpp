#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace common {
    struct rail_location_t {
        uint64_t id;
        rail_type_t type;

        static void serialise(std::span<uint8_t>& payload, const rail_location_t& data) {
            serialise_t::write(payload, data.id);
            serialise_t::write(payload, data.type);
        }

        static rail_location_t deserialise(std::span<uint8_t> payload) {
            rail_location_t result;

            result.id = serialise_t::read<uint64_t>(payload);
            result.type = serialise_t::read<rail_type_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                uint64_t, 
                uint8_t
            >();
        }
    };
}