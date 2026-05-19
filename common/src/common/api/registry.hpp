#pragma once

#include "common/core/packet_registry.hpp"

#include "common/messages/init_esp.hpp"
#include "common/messages/restart_esp.hpp"

namespace common {
    using registry_t = packet_registry_impl_t<
        init_esp_t,
        restart_esp_t
    >;

    // Globally provide a compile time size payload from registry.
    // std::array<uint8_t, {size_of_max_message_type}> 
    using payload_t = registry_t::payload_t;
    
    // The compile time id for each message.
    using packet_id_t = registry_t::packet_id_t;
}