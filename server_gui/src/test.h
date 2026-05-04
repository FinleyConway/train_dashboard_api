#include <iostream>

#include <span>
#include <cstdint>                                

#include "packet_registry.hpp"

struct engine_t {
    uint16_t next_speed;
    int8_t direction;

    #pragma pack(push, 1)
    struct net_t {
        uint16_t next_speed;
        int8_t direction;
    };
    #pragma pack(pop)

    static net_t to_net(engine_t data) {
        return {
            .next_speed = data.next_speed,
            .direction = data.direction
        };
    }

    static engine_t from_net(net_t net) {
        return {
            .next_speed = net.next_speed,
            .direction = net.direction
        };
    }
};

struct lights_t {
    uint8_t brightnes;

    #pragma pack(push, 1)
    struct net_t {
        uint8_t brightnes;
    };
    #pragma pack(pop)

    static net_t to_net(lights_t data) {
        return {
            .brightnes = data.brightnes,
        };
    }

    static lights_t from_net(net_t net) {
        return {
            .brightnes = net.brightnes,
        };
    }
};

void on_engine(const engine_t& engine) {
    std::cout << "Engine:\n";
    std::cout << (int)engine.next_speed << std::endl;
    std::cout << (int)engine.direction << std::endl;
}

void on_lights(const lights_t& lights) {
    std::cout << "Lights:\n";
    std::cout << (int)lights.brightnes << std::endl;
}

void send_interface(auto&& buffer) {

}

void recv_interface(auto buffer) {

}

int main() {
    packet_registry_impl_t<engine_t, lights_t> reg;
    reg.register_callback<engine_t, &on_engine>();
    reg.register_callback<lights_t, &on_lights>();

    engine_t temp = {
        .next_speed = 1000,
        .direction = 0
    };

    reg.send<engine_t>(temp, send_interface);

    reg.listen(recv_interface);
}