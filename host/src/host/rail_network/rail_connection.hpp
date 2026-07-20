#pragma once

#include "common/api/types.hpp"

namespace host {
    struct rail_connection_t {
        common::rail_id_t previous = 0; 
        common::rail_id_t next = 0;
        
        common::rail_type_t previous_type = common::rail_type_t::none; 
        common::rail_type_t next_type = common::rail_type_t::none;

        bool branch = false; // dont like this
    };
}