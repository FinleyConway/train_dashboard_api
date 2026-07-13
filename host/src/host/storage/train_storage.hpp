#pragma once

#include <unordered_map>

#include <nlohmann/json.hpp>

#include "common/api/types.hpp"
#include "common/messages/motor.hpp"

namespace host {
    struct storage_t {
        uint32_t current_motor_duty = 0;
        bool is_motor_active = false;

        uint16_t headlight_brightness = 0;
        bool is_headlight_active = false;
    };

    inline void to_json(nlohmann::json& j, const storage_t & e) {
        j = {
            {"current_motor_duty", e.current_motor_duty},
            {"is_motor_active", e.is_motor_active},
            {"headlight_brightness", e.headlight_brightness},
            {"is_headlight_active", e.is_headlight_active}
        };
    }

    class train_storage_t {
    public:
        void add_train(common::esp_id_t id) {
            m_data.try_emplace(id);
        }

        bool has_train(common::esp_id_t id) const {
            return m_data.contains(id);
        }

        void remove_train(common::esp_id_t id) {
            m_data.erase(id);
        }

        nlohmann::json get_train_json(common::esp_id_t id) const {
            auto it = m_data.find(id);
            if (it == m_data.end()) return nlohmann::json::object();

            return create_storage_json(id, it->second);
        }

        nlohmann::json get_all_train_json() const {
            nlohmann::json json = nlohmann::json::object();
            
            json["trains"] = nlohmann::json::array();

            for (const auto& [id, storage] : m_data) {
                json["trains"].emplace_back(create_storage_json(id, storage));
            }

            return json;
        }

    private:
        nlohmann::json create_storage_json(common::esp_id_t id, const storage_t& storage) const {
            nlohmann::json json = nlohmann::json::object();;

            json["train_id"] = id;
            json["train_status"] = storage;

            return json;
        } 

    private:
        std::unordered_map<common::esp_id_t, storage_t> m_data;
    };
}