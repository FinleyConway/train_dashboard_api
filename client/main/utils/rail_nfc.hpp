#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace client {
    struct rail_nfc_t {
        uint64_t rail_id;
        common::rail_type_t type;

        static rail_nfc_t deserialise(std::span<const uint8_t> payload) {
            rail_nfc_t result;

            result.rail_id = serialise_t::read<uint64_t>(payload);
            result.type = serialise_t::read<common::rail_type_t>(payload); 

            return result;
        }
    }
}