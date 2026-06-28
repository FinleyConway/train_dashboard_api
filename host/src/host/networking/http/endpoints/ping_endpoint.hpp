#pragma once

#include <httplib.h>

namespace host {
    class ping_endpoint_t {
    public:
        static void init(httplib::Server& http_server, const std::string& endpoint_uri) {
            http_server.Get(endpoint_uri, [&](const httplib::Request& req, httplib::Response& res) {
                res.status = 200;
                res.set_content(
                    R"({"status":"ok"})",
                    "application/json"
                );
            });
        }
    }
}