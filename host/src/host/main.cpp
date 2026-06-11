#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/utils/handshake_manager.hpp"
#include "host/networking/tcp_server.hpp"
#include "host/logging/logger.hpp"

#include "host/endpoints/train_status_endpoint.hpp"
#include "host/endpoints/motor_endpoint.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"

int main() {
    host::logger_t::init();

    host::tcp_server_t tcp_server;
    httplib::Server http_server;

    host::handshake_manager_t handshake(tcp_server, std::chrono::seconds(1));
    host::train_status_storage_t train_storage;

    tcp_server.register_on_connect([&](common::esp_id_t id) {
        handshake.on_connect(id);
        train_storage.add_train(id);
    });

    tcp_server.register_on_disconnect([&](common::esp_id_t id) {
        LOG_INFO("ESP: {} disconnected", id);

        train_storage.remove_train(id);
    });

    // setup tcp callbacks
    tcp_server.register_receive_callback<common::esp_init_response_t>([&](const common::esp_init_response_t& res) {
        handshake.on_response_received(res);
    });

    tcp_server.register_receive_callback<common::motor_status_t>([&](const common::motor_status_t& motor) {
        train_storage.update_motor_status(motor);
    });

    // setup http endpoints
    host::train_status_endpoint_t::init(train_storage, http_server, "/api/train_status");
    host::motor_endpoint_t::init(tcp_server, http_server, "/api/motor_control");

    // start communication
    tcp_server.start();
    http_server.listen("0.0.0.0", 8000); // blocks thread
}