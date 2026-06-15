#pragma once

#include <string>
#include <exception>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "host/storage/train_storage.hpp"
#include "host/networking/tcp/tcp_server.hpp"
#include "host/networking/http/http_utils.hpp"
#include "host/logging/logger.hpp"

#include "common/api/types.hpp"

namespace host {
    class train_endpoint_t {
    public:
        static void init(tcp_server_t& tcp_server, train_storage_t& storage, httplib::Server& http_server, const std::string& endpoint_uri) {
            http_server.Get(endpoint_uri + "/status", [&](const httplib::Request& req, httplib::Response& res) {
                get_all_status(storage, req, res);
            });

            http_server.Get(endpoint_uri + "/status/:train_id", [&](const httplib::Request& req, httplib::Response& res) {
                get_status(storage, req, res);
            });

            http_server.Post(endpoint_uri + "/start_polling", [&](const httplib::Request& req, httplib::Response& res) {
                start_polling(tcp_server, req, res);
            });

            http_server.Post(endpoint_uri + "/stop_polling", [&](const httplib::Request& req, httplib::Response& res) {
                stop_polling(tcp_server, req, res);
            });

            http_server.Post(endpoint_uri + "/disconnect/:train_id", [&](const httplib::Request& req, httplib::Response& res) {
                disconnect(tcp_server, req, res);
            });
        }

    private:
        static void get_status(train_storage_t& storage, const httplib::Request& req, httplib::Response& res) {
            try {
                auto train_id = http_utils_t::get_path_param<common::esp_id_t>(req, "train_id");

                if (!storage.has_train(train_id)) {
                    return http_utils_t::send(res, tcp_status_t::unknown_client);
                }

                res.set_content(
                    storage.get_train_json(train_id).dump(),
                    "application/json"
                );

                LOG_INFO("Getting train {} status", train_id);
            }
            catch (...) {
                http_utils_t::bad_request(res, "invalid train id");
            }
        }

        static void get_all_status(train_storage_t& storage, const httplib::Request& req, httplib::Response& res) {
            res.set_content(
                storage.get_all_train_json().dump(), 
                "application/json"
            );

            LOG_INFO("Getting all train status");
        }

        static void start_polling(tcp_server_t& tcp_server, const httplib::Request& req, httplib::Response& res) {
            http_utils_t::send(
                res, 
                tcp_server.toggle_accepting(true)
            );

            LOG_INFO("Starting train data polling");
        }

        static void stop_polling(tcp_server_t& tcp_server, const httplib::Request& req, httplib::Response& res) {
            http_utils_t::send(
                res, 
                tcp_server.toggle_accepting(false)
            );

            LOG_INFO("Stopping train data polling");
        }

        static void disconnect(tcp_server_t& tcp_server, const httplib::Request& req, httplib::Response& res) {
            try {
                auto train_id = http_utils_t::get_path_param<common::esp_id_t>(req, "train_id");
                
                http_utils_t::send(
                    res,
                    tcp_server.disconnect_client(train_id)
                );

                LOG_INFO("Disconnecting train: {}", train_id);
            }
            catch (...) {
                http_utils_t::bad_request(res, "invalid train id");
            }
        }
    };
}