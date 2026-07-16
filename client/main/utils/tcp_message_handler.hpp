#pragma once

#include "common/messages/rail_destination.hpp"
#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"

namespace client {
    struct system_bus_t;
    class tcp_client_t;

    class tcp_message_handler_t {
    public:
        static void init(system_bus_t& bus);

        static void register_messages(tcp_client_t& client);

    private:
        static void on_init_request(const common::esp_init_request_t& init_request);

        static void on_motor_control(const common::motor_control_t& motor_control);

        static void on_station_add(const common::rail_destination_t& rail_destination);

    private:
        static void handle_error();

    private:
        inline static system_bus_t* s_bus = nullptr;
    };
}