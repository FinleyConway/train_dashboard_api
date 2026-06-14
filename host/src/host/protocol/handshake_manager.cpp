#include "host/protocol/handshake_manager.hpp"

#include "host/logging/logger.hpp"

namespace host {
    handshake_manager_t::handshake_manager_t(tcp_server_t& tcp_server, const std::chrono::seconds& timeout) 
        : m_tcp_server(tcp_server), m_timeout(timeout) 
    {
    }

    void handshake_manager_t::on_connect(common::esp_id_t id) {
        // create a new handshake session
        auto [it, inserted] = m_session.try_emplace(
            id, asio::steady_timer(m_tcp_server.get_io_context(), m_timeout)
        );
        auto& session = it->second;

        // send the esp their given id
        m_tcp_server.send_to_client(id, common::esp_init_request_t {
            .id = id
        });

        // create a async timer that waits for the esp to respond or disconnect if they dont
        session.async_wait([this, id](const std::error_code& ec) {
            if (!ec) {
                LOG_WARN("ESP {}, handshake timeout", id);

                m_tcp_server.disconnect_client(id);
                m_session.erase(id);
            }
        });
    }

    void handshake_manager_t::on_response_received(const common::esp_init_response_t& res) {
        // check if the responded id exits
        auto it = m_session.find(res.id);
        if (it == m_session.end()) {
            LOG_WARN("Unknown id response: {}", res.id);
            return;    
        }

        // stop the session timer due to responded
        it->second.cancel();

        LOG_INFO("ESP {} connected", res.id);

        // clean up session
        m_session.erase(it);
    }         
}