#include "host/application.hpp"
#include "host/rail_network/rail_network.hpp"

int main() {
    // host::application_t app;
    // app.start();

    // return 0;

    host::rail_network_t network;

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 0,
            .type = common::rail_type_t::straight
        },
        .previous = {}
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 1,
            .type = common::rail_type_t::straight
        },
        .previous = host::rail_endpoint_t {
            .id = 0,
            .type = common::rail_type_t::straight
        }
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 2,
            .type = common::rail_type_t::right_junction
        },
        .previous = host::rail_endpoint_t {
            .id = 1,
            .type = common::rail_type_t::straight
        },
        .branch = host::rail_endpoint_t {
            .id = 10,
            .type = common::rail_type_t::right_curve
        }
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 11,
            .type = common::rail_type_t::straight
        },
        .previous = host::rail_endpoint_t {
            .id = 10,
            .type = common::rail_type_t::right_curve
        }
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 12,
            .type = common::rail_type_t::straight
        },
        .previous = host::rail_endpoint_t {
            .id = 11,
            .type = common::rail_type_t::straight
        }
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 3,
            .type = common::rail_type_t::straight
        },
        .previous = host::rail_endpoint_t {
            .id = 2,
            .type = common::rail_type_t::straight
        }
    });

    network.append_rail({
        .current = host::rail_endpoint_t {
            .id = 4,
            .type = common::rail_type_t::straight
        },
        .previous = host::rail_endpoint_t {
            .id = 3,
            .type = common::rail_type_t::straight
        }
    });

    std::vector<common::rail_id_t> path = network.generate_path(0, 12);

    for (auto rail : path) {
        std::cout << rail << std::endl;
    }
}