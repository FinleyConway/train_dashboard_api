#include "host/application.hpp"

#include "host/logging/logger.hpp"

#include "host/networking/http/endpoints/train_status_endpoint.hpp"
#include "host/networking/http/endpoints/motor_endpoint.hpp"

#include "common/messages/handshake.hpp"
#include "common/messages/motor.hpp"

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
            LOG_INFO("ESP: {} disconnected", id);

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
    }

    void application_t::register_endpoints() {
        train_status_endpoint_t::init(
            m_train_storage,
            m_http_server,
            "/api/train_status"
        );

        motor_endpoint_t::init(
            m_tcp_server,
            m_http_server,
            "/api/motor_control"
        );
    }
}