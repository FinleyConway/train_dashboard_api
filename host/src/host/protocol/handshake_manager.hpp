#pragma once

#include <chrono>
#include <unordered_map>

#include <asio.hpp>

#include "host/networking/tcp/tcp_server.hpp"

#include "common/api/types.hpp"

namespace host {
    /// @brief Handles the request and response between host and client
    class handshake_manager_t {
    public:
        /// @brief Construct handshake_manager_t
        /// @param tcp_server A mutable reference to the tcp_server
        /// @param timeout How long before disconnecting
        handshake_manager_t(tcp_server_t& tcp_server, const std::chrono::seconds& timeout);

        /// @brief Handle the connected client
        /// @param id The client id
        void on_connect(common::esp_id_t id);

        /// @brief The callback for the received acknowledgement
        /// @param res The acknowledgement message data
        void on_response_received(const common::esp_init_response_t& res);      

    private:
        tcp_server_t& m_tcp_server;
        std::unordered_map<common::esp_id_t, asio::steady_timer> m_session;
        std::chrono::seconds m_timeout;
    };
} 
