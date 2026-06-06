#pragma once

#include "common/core/serialise.hpp"

namespace common {
    struct motor_control_t {
        uint32_t starting_duty;
        uint32_t target_duty;
        uint16_t ramp_time_ms;
        uint8_t is_active;

        static void serialise(std::span<uint8_t>& payload, const motor_control_t& data) {
            serialise_t::write(payload, data.starting_duty);
            serialise_t::write(payload, data.target_duty);
            serialise_t::write(payload, data.ramp_time_ms);
            serialise_t::write(payload, data.is_active);
        }

        static motor_control_t deserialise(std::span<uint8_t> payload) {
            motor_control_t result;

            result.starting_duty = serialise_t::read<uint32_t>(payload);
            result.target_duty = serialise_t::read<uint32_t>(payload); 
            result.ramp_time_ms = serialise_t::read<uint16_t>(payload); 
            result.is_active = serialise_t::read<uint8_t>(payload);

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                uint32_t, 
                uint32_t, 
                uint16_t, 
                uint8_t
            >();
        }
    };
}