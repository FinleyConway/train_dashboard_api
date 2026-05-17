#include <iostream>

#include "networking/tcp_server.hpp"
#include "types/restart_esp.hpp"

void on_esp_init(common::init_esp_t) {

}

void on_connect(common::esp_id_t id) {
    std::cout << "ESP: " << id << " connected\n";
}

void on_disconnect(common::esp_id_t id) {
    std::cout << "ESP: " << id << " disconnected\n";
}

int main() {
    host::tcp_server_t server(ip::tcp::v4(), 8080);

    server.register_receive_callback<common::init_esp_t, &on_esp_init>();
    server.register_on_connect(on_connect);
    server.register_on_disconnect(on_disconnect);

    server.start();
    if (server.toggle_accepting(true) != host::tcp_status_t::success) {
        return -1;
    }

    while(server.is_running()) {
        std::string x;

        std::cout << "Input:\n";
        std::cin >> x;

        auto status = server.send_to_client(0, common::init_esp_t(1));

        if (status == host::tcp_status_t::success) {
            std::cout <<"sent !!\n";
        }

        if (status == host::tcp_status_t::unknown_client) {
            std::cout << "unknown client?\n";
        }

        if (status == host::tcp_status_t::no_client_connection) {
            std::cout << "no client\n";
        }
    }
}