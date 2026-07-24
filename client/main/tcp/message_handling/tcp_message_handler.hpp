#pragma once

#include "common/messages/rail_destination.hpp"
#include "common/messages/handshake.hpp"
#include "common/messages/motor_control.hpp"

namespace client {
    struct system_bus_t;
    class tcp_client_t;

    using connection_callback_t = void (*)(void*, common::esp_id_t);

    class tcp_message_handler_t {
    public:
        static void init(system_bus_t& bus, void* ctx, connection_callback_t callback);

        static void register_messages(tcp_client_t& client);

    private:
        static void on_init_request(const common::esp_init_request_t& init_request);

        static void on_motor_control(const common::motor_control_t& motor_control);

        static void on_station_add(const common::rail_destination_t& rail_destination);

    private:
        static void handle_error();

    private:
        inline static void* s_context = nullptr;
        inline static system_bus_t* s_bus = nullptr;
        inline static connection_callback_t s_connection_callback = nullptr;
    };
}