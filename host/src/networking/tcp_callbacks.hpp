#pragma once

#include <functional>

#include "registry.hpp"

namespace host {
    using on_connect_fn = std::function<void(common::esp_id_t)>;
    using on_disconnect_fn = std::function<void(common::esp_id_t)>;
}
