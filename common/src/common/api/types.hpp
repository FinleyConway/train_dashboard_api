#pragma once

#include <cstdint>

namespace common {
    using esp_id_t = uint16_t;

    enum class rail_type_t : uint8_t {
        horizontal = 0,
        vertical,
        left_curve,
        right_curve
    };
}