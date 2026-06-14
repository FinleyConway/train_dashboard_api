#pragma once

#include <httplib.h>

#include "host/protocol/handshake_manager.hpp"
#include "host/networking/tcp/tcp_server.hpp"
#include "host/storage/train_storage.hpp"

namespace host {
    class application_t {
    public:
        application_t();
        
        void start();

    private:
        void register_callbacks();
        void register_endpoints();

    private:
        tcp_server_t m_tcp_server;
        httplib::Server m_http_server;

        train_storage_t m_train_storage;
        handshake_manager_t m_handshake;
    };
}