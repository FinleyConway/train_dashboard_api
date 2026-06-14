#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/networking/tcp/tcp_server.hpp"
#include "host/protocol/http_utils.hpp"

#include "common/api/types.hpp"

namespace host {
    class motor_endpoint_t {
    public:
        static void init(tcp_server_t& tcp_server, httplib::Server& http_server, const std::string& endpoint_uri) {
            http_server.Post(endpoint_uri, [&](const httplib::Request& req, httplib::Response& res) {
                control_motor(tcp_server, req, res);
            });
        }

    private:
        static void control_motor(tcp_server_t& tcp_server, const httplib::Request& req, httplib::Response& res) {
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

                http_utils_t::send(res, status);
            }
            catch (const std::exception& e) {
                res.status = 400;
                res.set_content(
                    R"({"error":"invalid json payload"})",
                    "application/json"
                );
            }
        }
    };
}