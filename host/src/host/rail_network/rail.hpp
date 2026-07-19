#pragma once

#include <cstddef>

#include "common/api/types.hpp"

namespace host {
    class track_t;

    struct rail_branch_t {
        size_t track = 0;
        size_t position = 0;
    };

    struct rail_t {
        common::rail_id_t id;
        common::rail_type_t type;
        rail_branch_t branch;

        bool has_branch() const {
            return (
                type == common::rail_type_t::left_junction ||
                type == common::rail_type_t::right_junction
            );
        }
    };
}