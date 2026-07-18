#pragma once

#include <cstdint>

namespace common {
    using esp_id_t = uint16_t;
    using rail_id_t = uint64_t;

    enum class rail_type_t : uint8_t {
        none = 0,
        horizontal,
        left_curve,
        right_curve,
        left_junction,
        right_junction
    };
}