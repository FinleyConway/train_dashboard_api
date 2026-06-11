#pragma once

#include <unordered_map>

#include <nlohmann/json.hpp>

#include "common/api/types.hpp"
#include "common/messages/motor.hpp"

namespace host {
    struct esp_storage_t {
        common::motor_status_t motor;
    };

    inline void to_json(nlohmann::json& j, const esp_storage_t& e) {
        j = {
            {"current_motor_duty", e.motor.current_duty},
            {"is_motor_active", e.motor.is_active}
        };
    }

    class train_status_storage_t {
    public:
        void register_train(common::esp_id_t id) {
            m_data.emplace(id);
        }

        void update_motor_status(common::esp_id_t id, const common::motor_status_t& motor) {
            auto it = m_data.find(id);
            if (it == m_data.end()) return;

            it->second.motor = motor;
        }

        nlohmann::json get_train_json(common::esp_id_t id) {
            auto it = m_data.find(id);
            if (it == m_data.end()) return nlohmann::json();

            return create_storage_json(id, it->second);
        }

        nlohmann::json get_all_train_json() {
            nlohmann::json json;

            for (const auto& [id, storage] : m_data) {
                json.emplace_back(create_storage_json(id, storage));
            }

            return json;
        }

    private:
        nlohmann::json create_storage_json(common::esp_id_t id, const esp_storage_t& storage) {
            nlohmann::json json;

            json["train_id"] = id;
            json["train_status"] = storage;

            /*
            {
                "train_id" = 0,
                "train_status" = {
                    "current_motor_duty": 123,
                    "is_motor_active": 1,
                }
            }
            */

            return json;
        } 

    private:
        std::unordered_map<common::esp_id_t, esp_storage_t> m_data;
    };
}