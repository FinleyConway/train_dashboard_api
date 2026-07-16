#include "utils/tcp_message_handler.hpp"

#include <esp_log.h>

#include "system_bus.hpp"
#include "networking/tcp_client.hpp"
#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"
#include "common/messages/rail_destination.hpp"

namespace client {
    void tcp_message_handler_t::init(system_bus_t& bus) {
        s_bus = &bus;
    }

    void tcp_message_handler_t::register_messages(tcp_client_t& client) {
        client.register_receieve_callback<common::esp_init_request_t, &on_init_request>();
        client.register_receieve_callback<common::motor_control_t, &on_motor_control>();
        client.register_receieve_callback<common::rail_destination_t, &on_station_add>();
    }

    void tcp_message_handler_t::on_init_request(const common::esp_init_request_t& init_request) {
        handle_error();

        s_bus->outgoing_messages.send(tcp_event_data_t(
            common::esp_init_response_t {
                .id = init_request.id
            }
        ));

        s_bus->train_id = init_request.id;

        ESP_LOGI("tcp_message_handler", "Server ack, assigned id: %hu", init_request.id);
    }

    void tcp_message_handler_t::on_motor_control(const common::motor_control_t& motor_control) {
        handle_error();

        s_bus->motor_command.send(motor_control);
    }

    void tcp_message_handler_t::on_station_add(const common::rail_destination_t& rail_destination) {
        handle_error();

        s_bus->station_target.send(rail_destination.id);
    }

    void client::tcp_message_handler_t::handle_error() {
        if (s_bus == nullptr) {
            ESP_LOGE("tcp_message_handler", "System bus is null!");
            abort();
        }
    }
}
