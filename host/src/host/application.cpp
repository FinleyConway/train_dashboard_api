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

        m_tcp_server.register_receive_callback<common::motor_status_t>(
            [&](const common::motor_status_t& motor) {
                m_train_storage.update_motor_status(motor);
            }
        );

        m_tcp_server.register_receive_callback<common::headlight_status_t>(
            [&](const common::headlight_status_t& headlight) {
                m_train_storage.update_headlight_status(headlight);
            }
        );

        m_tcp_server.register_receive_callback<common::rail_location_t>(
            [&](const common::rail_location_t& location) {
                LOG_INFO("Rail id: {}, type: {}", location.id, static_cast<int>(location.type));
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
    }
}