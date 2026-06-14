#pragma once

#include <chrono>
#include <unordered_map>

#include <asio.hpp>

#include "host/networking/tcp/tcp_server.hpp"

#include "common/api/types.hpp"

namespace host {
    class handshake_manager_t {
    public:
        explicit handshake_manager_t(tcp_server_t& tcp_server, const std::chrono::seconds& timeout);

        void on_connect(common::esp_id_t id);

        void on_response_received(const common::esp_init_response_t& res);      

    private:
        tcp_server_t& m_tcp_server;
        std::unordered_map<common::esp_id_t, asio::steady_timer> m_session;
        std::chrono::seconds m_timeout;
    };
} 
