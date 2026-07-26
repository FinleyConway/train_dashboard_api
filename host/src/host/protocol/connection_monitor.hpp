#pragma once

#include <asio.hpp>

#include "host/networking/tcp/tcp_server.hpp"
#include "common/messages/heartbeat.hpp"

namespace host {
    class connection_monitor_t {
    public:
        connection_monitor_t(tcp_server_t& tcp_server, std::chrono::milliseconds poll_time) : 
            m_tcp_server(tcp_server), 
            m_timer(tcp_server.get_io_context()), 
            m_poll_time(poll_time)
        {
            check();
        }  
        
        void stop() {
            m_timer.cancel();
        }

    private:
        void check() {
            m_timer.expires_after(m_poll_time);
            m_timer.async_wait([this](const std::error_code& ec) {
                if (ec) return; // timer was cancelled

                m_tcp_server.send_to_all_clients(common::heartbeat_t());

                check();
            });
        }

    private:
        tcp_server_t& m_tcp_server;
        asio::steady_timer m_timer;
        std::chrono::milliseconds m_poll_time;
    };
}