#pragma once

#include <functional>

#include "common/core/net_types.hpp"

namespace host {
    using on_connect_fn = std::function<void(common::esp_id_t)>;
    using on_disconnect_fn = std::function<void(common::esp_id_t)>;
}
