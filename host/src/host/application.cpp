#include "host/application.hpp"

#include "host/logging/logger.hpp"

#include "host/networking/http/endpoints/ping_endpoint.hpp"
#include "host/networking/http/endpoints/train_endpoint.hpp"
#include "host/networking/http/endpoints/motor_endpoint.hpp"
#include "host/networking/http/endpoints/headlight_endpoint.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"
#include "common/messages/headlight.hpp"
#include "common/messages/rail_location.hpp"
#include "common/messages/rail_destination.hpp"
#include "common/messages/battery_status.hpp"

namespace host {
    application_t::application_t()
        : m_handshake(m_tcp_server, std::chrono::seconds(1))
    {
        logger_t::init();

        register_callbacks();
        register_endpoints();
    }

    void application_t::start() {
        m_tcp_server.start();
        m_tcp_server.toggle_accepting(true);
            
        m_http_server.listen("0.0.0.0", 8000);
    }

    void application_t::register_callbacks() {
        m_tcp_server.register_on_connect([&](common::esp_id_t id) {
            m_handshake.on_connect(id);
            m_train_storage.add_train(id);
        });

        m_tcp_server.register_on_disconnect([&](common::esp_id_t id) {
            m_train_storage.remove_train(id);
        });

        m_tcp_server.register_receive_callback<common::esp_init_response_t>(
            [&](const common::esp_init_response_t& res) {
                m_handshake.on_response_received(res);
            }
        );

        m_tcp_server.register_receive_callback<common::rail_location_t>(
            [&](const common::rail_location_t& location) {
                LOG_INFO("Train id: {}, Rail id: {}, type: {}", location.id, location.rail_id, static_cast<int>(location.type));
            }
        );

        m_tcp_server.register_receive_callback<common::battery_status_t>(
            [&](const common::battery_status_t& battery_status) {
                LOG_INFO("Train id: {}, Battery: {}", battery_status.id, battery_status.percentage_level);
            }
        );
    }

    void application_t::register_endpoints() {
        ping_endpoint_t::init(
            m_http_server,
            "/api/ping"
        );

        train_endpoint_t::init(
            m_tcp_server,
            m_train_storage,
            m_http_server,
            "/api/train"
        );

        motor_endpoint_t::init(
            m_tcp_server,
            m_http_server,
            "/api/motor_control"
        );

        headlight_endpoint_t::init(
            m_tcp_server,
            m_http_server,
            "/api/headlight_control"
        );

        m_http_server.Post("/api/station_request", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                const auto body = nlohmann::json::parse(req.body);

                const auto train_id   = body.at("train_id").get<common::esp_id_t>();
                const auto rail_id = body.at("rail_id").get<uint64_t>();

                /*
                {
                    "train_id": 0,
                    "rail_id": 280192301223,
                }
                */

                tcp_status_t status = m_tcp_server.send_to_client(train_id, common::rail_destination_t {
                    .id = rail_id
                });

                http_utils_t::send(res, status);

                LOG_INFO("Sending station request for train {}", train_id);
            }
            catch (...) {
                http_utils_t::bad_request(res, "invalid json payload");
            }
        });
    }
}