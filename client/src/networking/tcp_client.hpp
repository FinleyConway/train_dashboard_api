#pragma once

#include <sys/socket.h>
#include <netdb.h>

#include <esp_log.h>

#include "registry.hpp"

namespace client {
    enum class tcp_status_t {
        success,
        failure,
        connection_closed
    };

    class tcp_client_t {
    public:
        int get_socket() {
            return m_socket;
        }

        tcp_status_t connect_to(const char* address, const char* port) {
            struct addrinfo hints = {
                .ai_family = AF_INET,
                .ai_socktype = SOCK_STREAM
            };
            struct addrinfo* res;
            int error = getaddrinfo(address, port, &hints, &res);

            if (error == 0) {
                int socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

                if (socket_fd < 0) {
                    ESP_LOGE(s_tag, "Failed to create a socket");

                    return tcp_status_t::failure;
                }

                if (connect(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
                    ESP_LOGE(s_tag, "Failed to connect to server");

                    disconnect();

                    return tcp_status_t::failure;
                }

                m_socket = socket_fd;
            }

            return tcp_status_t::success;
        }

        tcp_status_t disconnect() {
            if (close(m_socket) < 0) {
                ESP_LOGE(s_tag, "Failed to close socket");

                return tcp_status_t::failure;
            }

            return tcp_status_t::success;
        }

        bool is_connected() const {
            return m_socket >= 0;
        }

        template<typename T, auto Fn>
        void register_receieve_callback() {
            m_registry.register_callback<T, Fn>();
        }

        template<typename T>
        tcp_status_t send_to_server(common::esp_id_t client_id, const T& data) {
            tcp_client_t status;

            m_registry.send<T>([&](common::payload_t&& buffer) {
                status = send_to(std::move(buffer));
            });

            return status;
        }

        tcp_status_t listen_to_server() {
            tcp_status_t status;

            m_registry.listen([&](common::payload_t& buffer) {
                size_t bytes_received = 0;

                status = receieve_from(buffer, bytes_received);

                return bytes_received;
            });

            return status;
        }

    private:
        tcp_status_t send_to(common::payload_t&& b) {
            common::payload_t buffer = std::move(b);
            size_t total_bytes_send = 0;

            while (total_bytes_send < buffer.size()) {
                ssize_t bytes_sent = send(
                    m_socket,
                    buffer.data() + total_bytes_send,
                    buffer.size() - total_bytes_send,
                    0
                );

                if (bytes_sent == -1) return tcp_status_t::failure;
                if (bytes_sent == 0) return tcp_status_t::connection_closed;

                total_bytes_send += bytes_sent;
            }

            return tcp_status_t::success;
        }

        tcp_status_t recv_exact(uint8_t* dst, size_t bytes_to_read, size_t& bytes_read) {
            bytes_read = 0;

            while (bytes_read < bytes_to_read) {
                ssize_t result = recv(
                    m_socket,
                    dst + bytes_read,
                    bytes_to_read - bytes_read,
                    0
                );

                if (result == -1) return tcp_status_t::failure;
                if (result == 0) return tcp_status_t::connection_closed;

                bytes_read += result;
            }

            return tcp_status_t::success;
        }

        tcp_status_t receieve_from(common::payload_t& buffer_out, size_t& bytes) {
            constexpr size_t esp_id_size = sizeof(common::esp_id_t);
            common::payload_t buffer;
            size_t bytes_read = 0;

            // receive the id from stream
            tcp_status_t status = recv_exact(
                buffer.data(), 
                esp_id_size, 
                bytes_read
            );

            if (status != tcp_status_t::success) return status;

            // get the payload size from the received id
            common::esp_id_t received_id = 0;
            std::memcpy(&received_id, buffer.data(), esp_id_size);

            size_t expected_bytes = m_registry.get_packet_bytes(received_id);
            size_t payload_bytes = 0;

            // read the remaining payload
            status = recv_exact(
                buffer.data() + bytes_read, 
                expected_bytes - bytes_read,
                payload_bytes
            );

            if (status != tcp_status_t::success) return status;

            bytes = bytes_read + payload_bytes;
            buffer_out = buffer;

            return tcp_status_t::success;
        }

    private:
        int m_socket = -1;
        common::registry_t m_registry;

        constexpr static const char* s_tag = "TCP";
    };
}