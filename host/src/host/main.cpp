#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/utils/handshake_manager.hpp"
#include "host/networking/tcp_server.hpp"
#include "host/logging/logger.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor_control.hpp"

void send_tcp_response(httplib::Response& res, host::tcp_status_t status) {
    switch (status) {
        case host::tcp_status_t::success:
            res.status = 200;
            res.set_content(
                R"({"status":"applied"})",
                "application/json"
            );
            break;

        default:
            res.status = 500;
            res.set_content(
                R"({"error":"tcp error"})",
                "application/json"
            );
            break;
    }
}

int main() {
    host::logger_t::init();
    host::tcp_server_t tcp_server;
    host::handshake_manager_t handshake(tcp_server, std::chrono::seconds(1));

    tcp_server.register_on_connect([&](common::esp_id_t id) {
        handshake.on_connect(id);
    });

    tcp_server.register_on_disconnect([&](common::esp_id_t id) {
        LOG_INFO("ESP: {} disconnected", id);
    });

    tcp_server.register_receive_callback<common::esp_init_response_t>([&](const common::esp_init_response_t& res) {
        handshake.on_response_received(res);
    });

    tcp_server.start();
    
    httplib::Server http_server;

    http_server.Post("/api/motor_control", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            const auto body = nlohmann::json::parse(req.body);

            const auto esp_id        = body.at("esp_id").get<common::esp_id_t>();
            const auto is_active     = body.at("is_active").get<bool>();
            const auto starting_duty = body.at("starting_duty").get<uint32_t>();
            const auto target_duty   = body.at("target_duty").get<uint32_t>();
            const auto ramp_time_ms  = body.at("ramp_time_ms").get<uint16_t>();

            /*
            {
                "esp_id": 0,
                "is_active": false,
                "starting_duty": 750,
                "target_duty": 1023,
                "ramp_time_ms": 7500
            }
            */

            host::tcp_status_t status = tcp_server.send_to_client(esp_id, common::motor_control_t {
                .starting_duty = starting_duty,
                .target_duty = target_duty,
                .ramp_time_ms = ramp_time_ms,
                .is_active = is_active
            });

            send_tcp_response(res, status);
        }
        catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                R"({"error":"invalid json payload"})",
                "application/json"
            );
        }
    });

    http_server.listen("0.0.0.0", 8081);
}