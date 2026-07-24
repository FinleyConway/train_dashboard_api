#pragma once

#include "host/rail_network/storage/rail_storage.hpp"
#include "host/rail_network/rail_connection.hpp"
#include "host/rail_network/rail_pathing.hpp"
#include "common/api/types.hpp"

namespace host {
    class rail_network_t {
    public:
        rail_network_t() : m_pathfinder(m_storage) {
            
        }

        rail_status_t append_rail(const rail_connection_t& connection) {
            // are we starting from a new track?
            if (!connection.has_rail_pivot()) {
                return m_storage.create_track(connection);
            }

            // does the rail reference point exist?
            if (!m_storage.exists(connection.previous.id)) {
                return rail_status_t::no_previous_rail;
            }

            // are we evaluating rails on the same track? 
            if (m_storage.exists(connection.next.id) && m_storage.exists(connection.previous.id))
                return m_storage.connect_tracks(connection);

            // just add rail to track
            return m_storage.append_to_track(connection);
        }

        std::optional<common::rail_id_t> get_next_rail(common::rail_id_t current) {
            return m_storage.get_next_rail(current);
        }

        std::vector<common::rail_id_t> generate_path(common::rail_id_t start, common::rail_id_t end) const {
            return m_pathfinder.generate_path(start, end);
        }

    private:
        rail_storage_t m_storage;
        rail_pathing_t m_pathfinder;
    };
}