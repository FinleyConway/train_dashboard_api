#include <iostream>

#include <span>
#include <cstdint>
#include <cstring>
#include <concepts>
#include <algorithm>                                     

// really should learn how to use macros....
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

template<typename T>
concept packet_type =
requires(T obj, typename T::net_t net) {
    typename T::net_t;

    { T::to_net(obj) } -> std::same_as<typename T::net_t>;
    { T::from_net(net) } -> std::same_as<T>;
}   
&& std::is_trivially_copyable_v<typename T::net_t>
&& std::is_standard_layout_v<typename T::net_t>;

template<packet_type... Ts>
struct packet_registry_t {
private:
    using packet_id_t = uint16_t;
    using callback_fn = void(*)(std::span<const uint8_t>);

public:
    template<typename T>
    auto send(int socket, const T& data) {
        static_assert(has_type<T>(), "T must be in Ts...");

        constexpr packet_id_t packet_id = get_type_id<T>();
        typename T::net_t net_data = T::to_net(data);

        std::array<uint8_t, get_max_bytes()> buffer;

        std::memcpy(buffer.data(), &packet_id, sizeof(packet_id_t)); // copy id to buffer
        std::memcpy(buffer.data() + sizeof(packet_id_t), &net_data, sizeof(typename T::net_t)); // copy net data to buffer

        // pretend to send bytes
        for (auto i : buffer) {
            std::cout << (int)i << std::endl;
        }

        return buffer;
    }

    void listen(int socket, std::span<const uint8_t> buffer) {
        //std::array<uint8_t, get_max_bytes()> buffer;

        // recv logic

        packet_id_t packet_id = 0;

        std::memcpy(&packet_id, buffer.data(), sizeof(packet_id_t));

        if (packet_id >= m_callback.size()) return; // prolly want to log this and/or return a status

        auto& callback = m_callback[packet_id];

        // call the callback if one has been registered
        if (callback.invoker != nullptr) {
            callback.invoker(std::span<const uint8_t>(
                buffer.data() + sizeof(packet_id_t), callback.bytes
            ));
        }
    }

    template<typename T, auto Fn>
    constexpr void register_callback() {
        static_assert(has_type<T>(), "T must be in Ts...");

        auto& callback = m_callback[get_type_id<T>()];
        
        callback.bytes = sizeof(typename T::net_t);
        callback.invoker = [](std::span<const uint8_t> bytes) {
            typename T::net_t net_data{};
            std::memcpy(&net_data, bytes.data(), sizeof(net_data));

            T obj = T::from_net(net_data);
            Fn(obj);
        };
    }

private:
    struct callback_info_t {
        callback_fn invoker = nullptr; 
        size_t bytes = 0;
    };

private:
    template<typename T, typename First, typename... Rest>
    static consteval size_t type_index_impl(size_t idx = 0) {
        if constexpr (std::is_same_v<T, First>)
            return idx;
        else {
            static_assert(sizeof...(Rest) > 0, "type not found");
            return type_index_impl<T, Rest...>(idx + 1);
        }
    }

    template<typename T>
    static consteval size_t get_type_id() {
        return type_index_impl<T, Ts...>();
    }

    static consteval size_t get_max_bytes() {
        return std::max({sizeof(typename Ts::net_t)...}) + sizeof(packet_id_t);
    }

    template<typename T>
    static consteval bool has_type() {
        return (std::is_same_v<T, Ts> || ...);
    }

private:
    std::array<callback_info_t, sizeof...(Ts)> m_callback;
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

int main() {
    packet_registry_t<engine_t, lights_t> reg;
    reg.register_callback<engine_t, &on_engine>();
    reg.register_callback<lights_t, &on_lights>();

    auto buffer = reg.send<engine_t>(0, engine_t {
        .next_speed = 1000,
        .direction = 0
    });

    reg.listen(0, std::span<const uint8_t>(buffer));
}