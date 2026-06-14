#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/storage/train_storage.hpp"
#include "host/networking/tcp/tcp_server.hpp"
#include "host/networking/http/http_utils.hpp"

#include "common/api/types.hpp"

namespace host {
    class train_status_endpoint_t {
    public:
        static void init(train_storage_t& storage, httplib::Server& http_server, const std::string& endpoint_uri) {
             http_server.Get(endpoint_uri, [&](const httplib::Request& req, httplib::Response& res) {
                get_all_status(storage, req, res);
            });

            http_server.Get(endpoint_uri + "/:train_id", [&](const httplib::Request& req, httplib::Response& res) {
                get_status(storage, req, res);
            });
        }

    private:
        static void get_status(train_storage_t& storage, const httplib::Request& req, httplib::Response& res) {
            try {
                std::string id_params = req.path_params.at("train_id");
                auto train_id = http_utils_t::to_integer<common::esp_id_t>(id_params);
                auto json = storage.get_train_json(train_id);

                res.set_content(json.dump(), "application/json");
            }
            catch (const std::exception& e) {
                res.status = 400;
                res.set_content(
                    R"({"error":"invalid train id"})",
                    "application/json"
                );
            }
        }

        static void get_all_status(train_storage_t& storage, const httplib::Request& req, httplib::Response& res) {
           auto json = storage.get_all_train_json();

            res.set_content(json.dump(), "application/json");
        }
    };
}