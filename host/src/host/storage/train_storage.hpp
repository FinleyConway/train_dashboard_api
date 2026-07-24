#pragma once

#include <optional>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "common/api/types.hpp"

namespace host {
    struct storage_t {
        float battery_level = 0.0f;
        std::optional<common::rail_id_t> last_track;
        std::optional<common::rail_type_t> last_track_type;
        uint32_t current_motor_duty = 0;
        bool is_motor_active = false;
    };

    inline void to_json(nlohmann::json& j, const storage_t & e) {
        j = {
            {"battery_level", e.battery_level},
            {"last_track", e.last_track},
            {"last_track_type", e.last_track_type},
            {"current_motor_duty", e.current_motor_duty},
            {"is_motor_active", e.is_motor_active},
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

        bool update_battery_level(common::esp_id_t id, float level) {
            auto it = m_data.find(id);
            if (it == m_data.end()) return false;

            it->second.battery_level = level;

            return true;
        }

        bool update_track_position(common::esp_id_t id, common::rail_id_t rail, common::rail_type_t type) {
            auto it = m_data.find(id);
            if (it == m_data.end()) return false;

            it->second.last_track = rail;
            it->second.last_track_type = type;

            return true;
        }

        bool update_motor_status(common::esp_id_t id, uint32_t duty, bool active) {
            auto it = m_data.find(id);
            if (it == m_data.end()) return false;

            it->second.current_motor_duty = duty;
            it->second.is_motor_active = active;

            return true;
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
            nlohmann::json json = nlohmann::json::object();

            json["train_id"] = id;
            json["train_status"] = storage;

            return json;
        } 

    private:
        std::unordered_map<common::esp_id_t, storage_t> m_data;
    };
}