#include "host/application.hpp"
#include "host/rail_network/rail_network.hpp"

int main() {
    //host::application_t app;
    //app.start();

    host::rail_network_t network;

    network.append_rail(host::rail_connection_t {
        .next = 0,
        .next_type = common::rail_type_t::straight,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 0,
        .next = 1,
        .previous_type = common::rail_type_t::straight,
        .next_type = common::rail_type_t::right_curve,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 1,
        .next = 2,
        .previous_type = common::rail_type_t::right_curve,
        .next_type = common::rail_type_t::straight,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 2,
        .next = 3,
        .previous_type = common::rail_type_t::straight,
        .next_type = common::rail_type_t::left_curve,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 3,
        .next = 4,
        .previous_type = common::rail_type_t::left_curve,
        .next_type = common::rail_type_t::straight,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 4,
        .next = 5,
        .previous_type = common::rail_type_t::straight,
        .next_type = common::rail_type_t::left_curve,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 5,
        .next = 6,
        .previous_type = common::rail_type_t::left_curve,
        .next_type = common::rail_type_t::straight,
    });

    network.append_rail(host::rail_connection_t {
        .previous = 6,
        .next = 0,
        .previous_type = common::rail_type_t::straight,
        .next_type = common::rail_type_t::right_curve,
    });

    auto rail_opt = network.get_next_rail(1);

    if (rail_opt.has_value()) {
        auto rail = rail_opt.value();

        std::cout << rail << std::endl;
    }
}