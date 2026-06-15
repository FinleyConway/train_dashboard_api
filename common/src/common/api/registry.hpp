#pragma once

#include "common/core/packet_registry.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"

namespace common {
    using registry_t = packet_registry_impl_t<
        esp_init_request_t,
        esp_init_response_t,
        motor_control_t,
        motor_status_t
    >;

    using payload_t = registry_t::payload_t;
    using packet_id_t = registry_t::packet_id_t;
}