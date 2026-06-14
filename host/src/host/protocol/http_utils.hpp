#pragma once

#include <charconv>

#include <httplib.h>

#include "host/networking/tcp/tcp_server.hpp"

namespace host {
    class http_utils_t {
    public:
        static void send(httplib::Response& res, host::tcp_status_t status) {
            switch (status) {
                case host::tcp_status_t::success:
                    res.status = 200;
                    res.set_content(
                        R"({"status":"applied"})",
                        "application/json"
                    );
                    break;

                default:
                    res.status = 500;
                    res.set_content(
                        R"({"error":"tcp error"})",
                        "application/json"
                    );
                    break;
            }
        } 

        template<typename T>
        static T to_integer(std::string_view s) {
            static_assert(std::is_integral_v<T>);

            T value{};
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

            if (ec != std::errc{} || ptr != s.data() + s.size())
                throw std::invalid_argument("invalid integer");

            return value;
        }
    };
}