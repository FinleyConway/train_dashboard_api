#pragma once

#include <optional>

#include "common/api/types.hpp"

namespace host {
    struct rail_endpoint_t {
        common::rail_id_t id;
        common::rail_type_t type = common::rail_type_t::none;
    };

    struct rail_connection_t {
        rail_endpoint_t next;
        rail_endpoint_t previous;
        std::optional<rail_endpoint_t> branch;

        rail_connection_t(rail_endpoint_t next, rail_endpoint_t previous, std::optional<rail_endpoint_t> branch = std::nullopt) : 
            next(next), 
            previous(previous), 
            branch(branch)
        {
        }

        bool has_rail_pivot() const {
            return previous.type != common::rail_type_t::none; 
        }
    };
}