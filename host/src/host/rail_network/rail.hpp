#pragma once

#include <cstddef>

#include "common/api/types.hpp"

namespace host {
    class track_t;

    struct rail_branch_t {
        size_t index = 0;
        size_t track_index = 0;
    };

    struct rail_t {
        common::rail_type_t type;
        rail_branch_t branch;
    };
}