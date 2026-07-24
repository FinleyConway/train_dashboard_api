#include "host/application.hpp"
#include "host/rail_network/rail_network.hpp"

int main() {
    // host::application_t app;
    // app.start();

    // return 0;

    host::rail_network_t network;

    std::vector<common::rail_id_t> path = network.generate_path(4, 3);

    for (auto rail : path) {
        std::cout << rail << std::endl;
    }
}