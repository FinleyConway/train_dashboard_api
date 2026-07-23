#pragma once

#include "common/messages/handshake.hpp"
#include "common/messages/rail_location.hpp"
#include "common/messages/battery_status.hpp"

#include "tcp/tcp_client.hpp"

namespace client {
    enum class tcp_event_type_t {
        init_respond,
        rail_location,
        motor_status,
        battery_status
    };

    struct tcp_event_data_t {
    private:
        // this should be fine as messages should only store trivial types
        union data_t {
            common::esp_init_response_t init_respond;
            common::rail_location_t rail_location;
            common::battery_status_t battery_status;
        };
    
    public:
        tcp_event_data_t() = default;

        explicit tcp_event_data_t(const common::esp_init_response_t& value) : m_type(tcp_event_type_t::init_respond) {
            m_data.init_respond = value;
        }

        explicit tcp_event_data_t(const common::rail_location_t& value) : m_type(tcp_event_type_t::rail_location) {
            m_data.rail_location = value;
        }

        explicit tcp_event_data_t(const common::battery_status_t& value) : m_type(tcp_event_type_t::battery_status) {
            m_data.battery_status = value;
        }

    public:
        tcp_event_type_t get_type() const {
            return m_type;
        }

        const data_t& get_data() const {
            return m_data;
        }

        tcp_status_t send_to_client(tcp_client_t& client) {
            switch (m_type) {
                case tcp_event_type_t::init_respond: return client.send_to_server(m_data.init_respond);
                case tcp_event_type_t::rail_location: return client.send_to_server(m_data.rail_location);
                case tcp_event_type_t::battery_status: return client.send_to_server(m_data.battery_status);
                default: return tcp_status_t::failure;
            }
        }

    private:
        tcp_event_type_t m_type;
        data_t m_data;
    };
}