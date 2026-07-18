#pragma once

#include <vector>
#include <optional>

#include "host/rail_network/rail.hpp"
#include "common/api/types.hpp"

namespace host {
    class track_t {
    public:
        explicit track_t(common::rail_type_t init_rail) {
            add(init_rail);
        }

        void add(common::rail_type_t type) {
            m_rails.emplace_back(type);
        }

        rail_t& get(size_t index) {
            return m_rails[index];
        }

        std::optional<size_t> get_track_connection() {
            return m_track_connection;
        }

        void set_track_connection(size_t track_index) {
            m_track_connection = track_index;
        }

    private:
        std::vector<rail_t> m_rails;

        // null -> no connection, end of rail
        // same index as track -> loops
        // different index -> moves to different track
        std::optional<size_t> m_track_connection;
    };
}