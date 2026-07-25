#pragma once

#include <optional>

#include "common/api/types.hpp"

namespace host {
    struct rail_endpoint_t {
        common::rail_id_t id;
        common::rail_type_t type = common::rail_type_t::none;

        bool is_junction() const {
            return type == common::rail_type_t::left_junction ||
                   type == common::rail_type_t::right_junction;
        }
    };

    struct rail_connection_t {
        rail_endpoint_t current;
        rail_endpoint_t previous;
        std::optional<rail_endpoint_t> branch;

        bool has_rail_pivot() const {
            return previous.type != common::rail_type_t::none; 
        }

        bool has_branch() const {
            return branch.has_value();
        }
    };
}