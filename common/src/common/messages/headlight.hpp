#pragma once

#include "common/core/serialise.hpp"

namespace common {
    struct headlight_control_t {
        uint16_t brightness;
        uint8_t is_active;

        static void serialise(std::span<uint8_t>& payload, const headlight_control_t& data) {
            serialise_t::write(payload, data.brightness);
            serialise_t::write(payload, data.is_active);
        }

        static headlight_control_t deserialise(std::span<const  uint8_t> payload) {
            headlight_control_t result;

            result.brightness = serialise_t::read<uint16_t>(payload);
            result.is_active = serialise_t::read<uint8_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                uint16_t, 
                uint8_t
            >();
        }
    };

    struct headlight_status_t {
        common::esp_id_t id;
        uint16_t brightness;
        uint8_t is_active;

        static void serialise(std::span<uint8_t>& payload, const headlight_status_t& data) {
            serialise_t::write(payload, data.id);
            serialise_t::write(payload, data.brightness);
            serialise_t::write(payload, data.is_active);
        }

        static headlight_status_t deserialise(std::span<const uint8_t> payload) {
            headlight_status_t result;

            result.id = serialise_t::read<common::esp_id_t>(payload);
            result.brightness = serialise_t::read<uint16_t>(payload);
            result.is_active = serialise_t::read<uint8_t>(payload);

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<
                common::esp_id_t,
                uint16_t, 
                uint8_t
            >();
        }
    };
}