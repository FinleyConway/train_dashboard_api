#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/networking/tcp/tcp_server.hpp"
#include "host/networking/http/http_utils.hpp"
#include "host/logging/logger.hpp"

#include "common/api/types.hpp"
#include "common/messages/headlight.hpp"

namespace host {
    class headlight_endpoint_t {
    public:
        static void init(tcp_server_t& tcp_server, httplib::Server& http_server, const std::string& endpoint_uri) {
            http_server.Post(endpoint_uri, [&](const httplib::Request& req, httplib::Response& res) {
                control_headlight(tcp_server, req, res);
            });
        }

    private:
        static void control_headlight(tcp_server_t& tcp_server, const httplib::Request& req, httplib::Response& res) {
            try {
                const auto body = nlohmann::json::parse(req.body);

                const auto train_id   = body.at("train_id").get<common::esp_id_t>();
                const auto brightness = body.at("brightness").get<uint16_t>();
                const auto is_active  = body.at("is_active").get<bool>();

                /*
                {
                    "train_id": 0,
                    "brightness": 500,
                    "is_active": false
                }
                */

                tcp_status_t status = tcp_server.send_to_client(train_id, common::headlight_control_t {
                    .brightness = brightness,
                    .is_active = is_active
                });

                http_utils_t::send(res, status);

                LOG_INFO("Sending headlight request for train {}", train_id);
            }
            catch (...) {
                http_utils_t::bad_request(res, "invalid json payload");
            }
        }
    };
}