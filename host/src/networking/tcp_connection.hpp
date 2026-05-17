#pragma once

#include <memory>

#include <asio.hpp>

#include "tcp_callbacks.hpp"
#include "tcp_io_state.hpp"
#include "registry.hpp"

namespace ip = asio::ip;

namespace host {
    class tcp_connection_t : public std::enable_shared_from_this<tcp_connection_t> {
    public:
        using pointer_t = std::shared_ptr<tcp_connection_t>;

        static pointer_t create(asio::io_context& io_context) {
            return pointer_t(new tcp_connection_t(io_context));
        }

        ip::tcp::socket& get_socket() {
            return m_socket;
        }

        void set_spec(common::esp_id_t id, common::registry_t& registry, on_disconnect_fn&& callback) {
            m_id = id;
            m_io_state.set_spec(registry, [this](common::esp_id_t) {
                disconnect();
            });
            m_disconnect_callback = std::move(callback);
        }

        bool send(common::payload_t&& payload, size_t bytes) {
            return m_io_state.send(std::move(payload), bytes);
        }

        void set_receiving_state(bool enable) {
            if (enable) {
                m_io_state.start_receiving();
            }   
            else {
                m_io_state.stop_receiving();
            }
        }

        bool disconnect() {
            if (!m_socket.is_open()) return false;

            if (m_disconnect_callback) {
                m_disconnect_callback(m_id);
            }

            m_io_state.stop_io();
            m_socket.close();

            return true;
        }

    private:
        explicit tcp_connection_t(asio::io_context& io_context) : m_socket(io_context), m_io_state(m_socket) {
        }

    private:
        ip::tcp::socket m_socket;
        tcp_io_state_t m_io_state;

        common::esp_id_t m_id = 0;
        on_disconnect_fn m_disconnect_callback;
    };

    using tcp_connection_ptr_t = tcp_connection_t::pointer_t;
}