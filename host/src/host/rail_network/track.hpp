#pragma once

#include <vector>
#include <variant>
#include <optional>

#include "host/rail_network/rail.hpp"
#include "common/api/types.hpp"

namespace host {
    class track_t {
    public:
        explicit track_t(common::rail_id_t id, common::rail_type_t type) {
            add(id, type);
        }

        void add(common::rail_id_t id, common::rail_type_t type) {
            m_rails.emplace_back(rail_t {
                .id = id,
                .type = type
            });
        }

        bool has(size_t position) const {
            return m_rails.size() > position;
        }

        const rail_t& get(size_t position) const {
            return m_rails[position];
        }

        bool add_branch(size_t index, const rail_branch_t& branch) {
            if (index >= m_rails.size()) {
                return false;
            }

            m_rails.at(index).branch = branch;

            return true;
        }

        std::optional<size_t> get_track_connections() const {
            return m_track_connection;
        }

        void set_track_connection(size_t track_index) {
            m_track_connection = track_index;
        }

        size_t size() const {
            return m_rails.size();
        }

    private:
        std::vector<rail_t> m_rails;

        // null -> no connection, end of rail
        // same index as track -> loops
        // different index -> moves to different track
        std::optional<size_t> m_track_connection;
    };
}