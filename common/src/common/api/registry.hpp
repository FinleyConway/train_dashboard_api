#pragma once

#include "common/core/packet_registry.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor_control.hpp"
#include "common/messages/motor_status.hpp"
#include "common/messages/rail_location.hpp"
#include "common/messages/rail_destination.hpp"
#include "common/messages/battery_status.hpp"

namespace common {
    using registry_t = packet_registry_impl_t<
        esp_init_request_t,
        esp_init_response_t,
        motor_control_t,
        motor_status_t,
        rail_location_t,
        rail_destination_t,
        battery_status_t
    >;

    using payload_t = registry_t::payload_t;
    using packet_id_t = registry_t::packet_id_t;
}