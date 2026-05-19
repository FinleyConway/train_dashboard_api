#include <catch2/catch_test_macros.hpp>

#include <bit>

#include "common/core/serialisation.hpp"

struct test_t {
    int64_t id = 0;
    uint32_t sweets = 0;
    int16_t lollies = 0;
    uint8_t pop = 0;

    bool operator==(const test_t& other) const {
        return (
            id == other.id &&
            sweets == other.sweets &&
            lollies == other.lollies &&
            pop == other.pop
        );
    }
};

TEST_CASE( "Integer serialisation", "[serialisation]" ) {
    std::array<uint8_t, 10> payload;

    SECTION("Write to payload") {
        std::span<uint8_t> payload_sp(payload);
        uint16_t value = 10;

        common::write_int(payload_sp, value);

        uint16_t test_value = 0;
        std::memcpy(&test_value, payload.data(), sizeof(uint16_t));

        if constexpr (std::endian::native == std::endian::big) {
            REQUIRE(test_value == 0x0A00);
        }
        else {
            REQUIRE(test_value == 0x000A);
        }
    }

    SECTION("Read from payload") {
        payload[0] = 0x0A;
        payload[1] = 0x00;

        std::span<uint8_t> payload_sp(payload);
        uint16_t value = common::read_int<uint16_t>(payload_sp);

        REQUIRE(value == 10);
    }

    SECTION("Read and writing from payload") {
        test_t write_t = {
            .id = 100,
            .sweets = 99,
            .lollies = 50,
            .pop =2
        };

        std::array<uint8_t, 15> payload;
        std::span<uint8_t> payload_sp(payload);

        common::write_int(payload_sp, write_t.id);
        common::write_int(payload_sp, write_t.sweets);
        common::write_int(payload_sp, write_t.lollies);
        common::write_int(payload_sp, write_t.pop);

        test_t send_t;

        send_t.id = common::read_int<int64_t>(payload_sp);
        send_t.sweets = common::read_int<uint32_t>(payload_sp);
        send_t.lollies = common::read_int<int16_t>(payload_sp);
        send_t.pop = common::read_int<uint8_t>(payload_sp);

        REQUIRE(write_t == send_t);
    }
}