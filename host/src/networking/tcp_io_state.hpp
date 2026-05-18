#pragma once

#include <deque>
#include <cstdint>
#include <functional>
#include <system_error>

#include <asio.hpp>

#include "registry.hpp"
#include "logging/logger.hpp"
#include "logging/assert.hpp"
#include "networking/tcp_callbacks.hpp"

namespace host {
    namespace ip = asio::ip;

    class tcp_io_state_t {
    public:
        tcp_io_state_t(ip::tcp::socket& io_context) : m_socket(io_context) {}

        void set_spec(common::registry_t& registry, on_disconnect_fn&& callback) {
            m_registry = &registry;
            m_disconnect_callback = std::move(callback);
        }

        bool send(common::payload_t&& payload, size_t bytes) {
            if (!m_socket.is_open()) return false;

            m_write_state_queue.emplace_back(
                std::move(payload),
                bytes
            );
            
            if (m_io_write_state == io_state_t::idle) {
                write();
            }

            return true;
        }

        void start_receiving() {
            m_can_receive = true;

            read_id();
        }

        void stop_receiving() {
            m_can_receive = false;
            m_io_read_state == io_state_t::idle;
        }

        void stop_io() {
            m_can_receive = false;
            m_io_read_state == io_state_t::idle;
            m_io_write_state = io_state_t::idle;
            m_write_state_queue.clear();
        }

    private:
        enum class io_state_t {
            idle,
            reading_payload,
            writing_payload
        };

    private:
        void write() {
            if (m_write_state_queue.empty()) return;
            if (m_io_write_state == io_state_t::writing_payload) return;

            m_io_write_state = io_state_t::writing_payload;
            auto& buffer = m_write_state_queue.front();

            asio::async_write(
                m_socket, 
                asio::buffer(buffer.payload.data(), buffer.bytes),
                [this](const std::error_code& ec, size_t) {
                    m_io_write_state = io_state_t::idle;

                    if (!has_or_handle_io_error(ec)) {
                        // remove currently sent state
                        m_write_state_queue.pop_front();

                        // start the next write cycle
                        write();
                    }
                }
            );
        }

        void read_id() {    
            if (!m_can_receive) return;
            if (m_io_read_state == io_state_t::reading_payload) return;

            m_io_read_state = io_state_t::reading_payload;

            asio::async_read(
                m_socket,
                asio::buffer(&m_read_state.id, sizeof(common::esp_id_t)),
                [this](const std::error_code& ec, size_t bytes_transferred) {
                    read_payload(ec, bytes_transferred);
                }
            );
        }

        void read_payload(const std::error_code& id_ec, size_t) {
            if (has_or_handle_io_error(id_ec)) return;

            LOG_ASSERT(m_registry != nullptr, "m_registry is nullptr!");

            size_t size = m_registry->get_packet_bytes(m_read_state.id);

            LOG_ASSERT(size != 0, "Packet size is not registered!");

            asio::async_read(
                m_socket,
                asio::buffer(m_read_state.payload.data(), size),
                [this](const std::error_code& ec, size_t bytes_transferred) {
                    if (!has_or_handle_io_error(ec)) {
                        LOG_ASSERT(m_registry != nullptr, "m_registry is nullptr!");

                        // notify server
                        m_registry->dispatch(
                            m_read_state.id,
                            std::move(m_read_state.payload), 
                            bytes_transferred
                        );

                        // start the next read cycle
                        m_io_read_state = io_state_t::idle;
                        read_id();
                    }
                }
            );
        }

        bool has_or_handle_io_error(const std::error_code& ec) {
            // all errors seem to be suitable to just disconnect client
            if (ec) {
                LOG_ERROR("Failed to read/write: {}", ec.message());

                stop_io();

                if (m_disconnect_callback) {
                    m_disconnect_callback(0); // id is pointless here, kind of a code smell?
                }

                return true;
            }

            return false;
        }

    private:
        struct write_state_t {
            const common::payload_t payload;
            const size_t bytes = 0;
        };

        struct read_state_t {
            common::esp_id_t id = 0;
            common::payload_t payload;
        };

    private:
        ip::tcp::socket& m_socket;
        common::registry_t* m_registry = nullptr;

        bool m_can_receive = false;
        read_state_t m_read_state;
        std::deque<write_state_t> m_write_state_queue;
        io_state_t m_io_read_state = io_state_t::idle;
        io_state_t m_io_write_state = io_state_t::idle;

        on_disconnect_fn m_disconnect_callback;
    };
}