#pragma once

#include <freertos/FreeRTOS.h>

#include "common/messages/rail_destination.hpp"
#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"

namespace client {
    class tcp_client_t;

    class tcp_manager_task_t {
    private:
        using connection_callback_t = void(*)(common::esp_id_t);

    public:
        static void init(connection_callback_t&& callback);

        static TaskHandle_t get_handle();

    private:
        static void run(void* parameters);

        static void register_messages(tcp_client_t& client);

        static void try_connect(tcp_client_t& client);
    
    private:
        static void on_init_request(const common::esp_init_request_t& init_request);

        static void on_motor_control(const common::motor_control_t& motor_control);

        static void on_station_add(const common::rail_destination_t& rail_destination);

    private:
        static constexpr const char* c_tag = "tcp_task";
        inline static connection_callback_t s_connection_callback = nullptr;
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 8192;
    };
}