#pragma once

#include <array>

namespace common {
    struct service_entry_t {
        const char* service_type;
        const char* hostname;
        const char* port;

        constexpr service_entry_t(const char* service_type, const char* hostname, const char* port) 
            : service_type(service_type), hostname(hostname), port(port) 
        {
        }
    };

    struct service_config_t {
        // each service_type name must be unique!
        static constexpr std::array services {
            service_entry_t("esp-tcp", "esp_server.local", "8080"),
            service_entry_t("http", "esp-api.local", "8000"),
        };

        // for tcp server/client to easily get
        static constexpr const char* tcp_hostname = services[0].hostname;
        static constexpr const char* tcp_port = services[0].port;
    };
}