#pragma once

#include <type_traits>
#include <cstdint>
#include <cstring>
#include <bit>

#include "common/core/net_types.hpp"

namespace common {
    template <typename T>
    struct is_fixed_width_int : std::bool_constant<
        std::is_same_v<T, int8_t>  ||
        std::is_same_v<T, uint8_t> ||
        std::is_same_v<T, int16_t> ||
        std::is_same_v<T, uint16_t>||
        std::is_same_v<T, int32_t> ||
        std::is_same_v<T, uint32_t>||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, uint64_t>
    > {};

    template<typename TInt>
    void write_int(common::payload_view_t& payload, TInt int_type) {
        static_assert(is_fixed_width_int<TInt>::value, "write_int T must be a fixed width integer type!");

        constexpr size_t size = sizeof(TInt);
        constexpr bool is_big_endian = std::endian::native == std::endian::big;

        // if the systems is big then swap to turn small
        // since esp is little
        if constexpr (is_big_endian && size > 1) {
            int_type = std::byteswap(int_type);
        }

        std::memcpy(payload.data(), &int_type, size);

        payload = payload.subspan(size); // offset payload
    }

    template<typename TInt>
    TInt read_int(common::payload_view_t& payload) {
        static_assert(is_fixed_width_int<TInt>::value, "write_int T must be a fixed width integer type!");

        constexpr size_t size = sizeof(TInt);
        constexpr bool is_big_endian = std::endian::native == std::endian::big;

        TInt int_type = 0;

        std::memcpy(&int_type, payload.data(), size);

        // if the systems is big then swap to turn small
        // since esp is little
        if constexpr (is_big_endian && size > 1) {
            int_type = std::byteswap(int_type);
        }

        payload = payload.subspan(size); // offset payload

        return int_type;
    }
}