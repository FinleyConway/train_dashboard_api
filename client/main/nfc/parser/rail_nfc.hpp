#pragma once

#include "common/core/serialise.hpp"
#include "common/api/types.hpp"

namespace client {
    struct rail_nfc_t {
        common::rail_id_t rail_id;
        common::rail_type_t type;

        static rail_nfc_t deserialise(std::span<const uint8_t> payload) {
            rail_nfc_t result;

            result.rail_id = common::serialise_t::read<common::rail_id_t>(payload);
            result.type = common::serialise_t::read<common::rail_type_t>(payload); 

            return result;
        }
    };
}