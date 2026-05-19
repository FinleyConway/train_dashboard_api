#pragma once

#include "common/core/packet_registry.hpp"

#include "common/messages/init_esp.hpp"
#include "common/messages/restart_esp.hpp"

namespace common {
    using registry_t = packet_registry_impl_t<
        init_esp_t,
        restart_esp_t
    >;
}