#include <catch2/catch_test_macros.hpp>

#include "common/packet_registry.hpp"

struct test_one_t {
    uint16_t id = 0;

    #pragma pack(push, 1)
    struct net_t {
        uint16_t id;
    };
    #pragma pack(pop)

    static net_t to_net(test_one_t data) {
        return {
            .id = data.id
        };
    }

    static test_one_t from_net(net_t net) {
        return {
            .id = net.id
        };
    }
};

struct test_two_t {
    uint16_t id = 0;

    #pragma pack(push, 1)
    struct net_t {
        uint16_t id;
    };
    #pragma pack(pop)

    static net_t to_net(test_two_t data) {
        return {
            .id = data.id
        };
    }

    static test_two_t from_net(net_t net) {
        return {
            .id = net.id
        };
    }
};

using reg_t = common::packet_registry_impl_t<
    test_one_t,
    test_two_t
>;

using payload_t = reg_t::payload_t;
using reg_id_t = reg_t::packet_id_t;

void test_one_callback(const test_one_t& t) {
    REQUIRE(t.id == 10);
}

TEST_CASE( "packet_registry class", "[packet_registry]" ) {
    reg_t registry;

    SECTION("Get packet size") {
        size_t bytes_1 = registry.get_packet_bytes(0);
        size_t bytes_2 = registry.get_packet_bytes<test_one_t>();

        REQUIRE(bytes_1 == 0);
        REQUIRE(bytes_2 == 0);

        registry.register_callback<test_one_t, &test_one_callback>();

        size_t reg_bytes_1 = registry.get_packet_bytes(0);
        size_t reg_bytes_2 = registry.get_packet_bytes<test_one_t>();

        REQUIRE(reg_bytes_1 == 2);
        REQUIRE(reg_bytes_2 == 2);
    }

    SECTION("Create payload") {
        test_one_t input{ .id = 10 };

        payload_t payload = registry.create_payload(input);

        reg_id_t extracted_id = 0;
        std::memcpy(&extracted_id, payload.data(), sizeof(reg_id_t));

        REQUIRE(extracted_id == 0);

        test_one_t::net_t extracted_data;
        std::memcpy(
            &extracted_data,
            payload.data() + sizeof(reg_id_t),
            sizeof(extracted_data)
        );

        test_one_t::net_t expected = test_one_t::to_net(input);

        REQUIRE(extracted_data.id == expected.id);
    }

    SECTION("Dispatch regisered payload") {
        registry.register_callback<test_one_t, &test_one_callback>();
        
        test_one_t input{ .id = 10 };
        payload_t payload = registry.create_payload(input);

        bool result = registry.dispatch(
            0,
            std::move(payload),
            4
        );

        REQUIRE(result == true);
    }

    SECTION("Dispatch invalid payload") {
        bool result = registry.dispatch(
            999,
            payload_t{},
            0
        );

        REQUIRE(result == false);
    }

    SECTION("Dispatch unregisered payload") {
        test_one_t input{ .id = 10 };
        payload_t payload = registry.create_payload(input);

        bool result = registry.dispatch(
            0,
            std::move(payload),
            4
        );

        REQUIRE(result == false);
    }

    SECTION("Dispatch payload size mismatch") {
        registry.register_callback<test_one_t, &test_one_callback>();

        test_one_t input{ .id = 10 };
        payload_t payload = registry.create_payload(input);

        bool result = registry.dispatch(
            0,
            std::move(payload),
            4 + 5
        );

        REQUIRE(result == false);
    }
}