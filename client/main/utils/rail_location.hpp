#pragma once

#include "common/api/types.hpp"
#include "common/core/serialise.hpp"

namespace client {
    struct rail_location_t {
        uint64_t id;
        common::rail_type_t type;

        static rail_location_t deserialise(std::span<const uint8_t> payload) {
            rail_location_t result;
            
            result.id = common::serialise_t::read<uint64_t>(payload);
            result.type = common::serialise_t::read<common::rail_type_t>(payload); 

            return result;
        }
    };
}