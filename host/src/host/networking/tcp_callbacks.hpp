#pragma once

#include <functional>

#include "common/api/types.hpp"

namespace host {
    using on_connect_fn = std::function<void(common::esp_id_t)>;
    using on_disconnect_fn = std::function<void(common::esp_id_t)>;
}
