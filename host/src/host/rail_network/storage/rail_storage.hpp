#pragma once

#include <vector>
#include <algorithm>
#include <unordered_map>

#include "common/api/types.hpp"
#include "host/rail_network/storage/track.hpp"
#include "host/rail_network/rail_connection.hpp"

namespace host {
    enum class rail_status_t {
        success,
        rail_already_exists,
        no_previous_rail,
        invalid_connection,
        invalid_loop
    };

    // basically a directed graph?
    class rail_storage_t {
    private:
        struct location_t {
            size_t track = 0;
            size_t position = 0;
        };

    public:
        bool exists(common::rail_id_t id) const {
            return m_rail_lookup.contains(id);
        }

        const rail_t& get_from_id(common::rail_id_t id) const {
            const location_t& location = m_rail_lookup.at(id);

            return m_tracks[location.track].get(location.position);
        }

        const rail_t& get_from_branch(const rail_branch_t& branch) const {
            return m_tracks[branch.track].get(branch.position);
        }

        rail_status_t create_track(const rail_endpoint_t& endpoint) {
            if (m_rail_lookup.contains(endpoint.id)) {
                return rail_status_t::rail_already_exists;
            }

            // create a new track with an inital track piece
            m_tracks.emplace_back(endpoint);

            // add rail
            m_rail_lookup.emplace(endpoint.id, location_t {
                .track = m_tracks.size() - 1,
                .position = 0 // first rail in the newly created track
            });

            return rail_status_t::success;
        }

        rail_status_t append_to_track(const rail_connection_t& connection) {
            if (exists(connection.current.id)) {
                return rail_status_t::rail_already_exists;
            }

            const location_t& location = m_rail_lookup.at(connection.previous.id);
            track_t& track = m_tracks[location.track];
            
            track.add(connection.current);

            const auto& current_location = m_rail_lookup.emplace(
                connection.current.id, 
                location_t {
                    .track = location.track,
                    .position = track.size() - 1
                }
            );

            if (connection.has_branch()) {
                return create_branch(
                    current_location.first->second, // get location_t from created track
                    connection
                );
            }

            return rail_status_t::success;
        }

        rail_status_t connect_tracks(const rail_connection_t& connection) {
            const location_t& current = m_rail_lookup.at(connection.current.id);
            const location_t& previous = m_rail_lookup.at(connection.previous.id);

            // it loops
            if (current.track == previous.track) {
                track_t& track = m_tracks[current.track];
                const auto [first, last] = std::minmax(current.position, previous.position);

                if (first != 0 && last != track.size() - 1) {
                    return rail_status_t::invalid_loop;
                }

                // tell to look at the same track
                m_tracks[current.track].set_track_connection(current.track);

                return rail_status_t::success;
            }
            
            // tell to look at another track
            track_t& track = m_tracks[previous.track];
            track.set_track_connection(current.track);
            track.add_branch(previous.position, rail_branch_t {
                .track = current.track,
                .position = current.position
            });

            return rail_status_t::success;
        }
        
        std::optional<common::rail_id_t> get_next_rail(common::rail_id_t current_rail) const {
            auto it = m_rail_lookup.find(current_rail);
            if (it == m_rail_lookup.end()) return std::nullopt;

            const location_t& location = it->second;
            const track_t& track = m_tracks[location.track];

            // try current rail on the same track
            const size_t location_offset = location.position + 1;
            if (track.has(location_offset)) {
                return track.get(location_offset).id;
            }

            // no more rails on this track, try connection
            const std::optional<size_t> connection = track.get_track_connections();
            if (!connection.has_value()) {
                return std::nullopt;
            }

            const size_t connected_track_index = connection.value();
            const host::track_t& connected_track = m_tracks[connected_track_index];

            // is the track a loop?
            if (connected_track_index == location.track) {
                if (connected_track.has(0)) {
                    return connected_track.get(0).id;
                }

                return std::nullopt;
            }

            // get the branching track
            if (track.has(location.position)) {
                const rail_t& rail = track.get(location.position);
                const rail_branch_t& branch = rail.branch;

                if (connected_track.has(branch.position)) {
                    return connected_track.get(branch.position).id;
                }
            }

            return std::nullopt;
        }

    private:   
        rail_status_t create_branch(const location_t& location, const rail_connection_t& connection) {
            // can the current type actually branch?
            if (!connection.current.is_junction()) return rail_status_t::invalid_connection;

            const rail_endpoint_t& branch = connection.branch.value();

            rail_status_t ret = create_track(branch);
            if (ret != rail_status_t::success) {
                return ret;
            }

            bool success = m_tracks[location.track].add_branch(
                location.position, 
                rail_branch_t {
                    .track = m_tracks.size() - 1,
                    .position = 0 // first rail in the newly created track
                }
            );
            
            if (!success) {
                return rail_status_t::invalid_connection;
            }


            return rail_status_t::success;
        }

    private:
        std::vector<track_t> m_tracks;
        std::unordered_map<common::rail_id_t, location_t> m_rail_lookup;
    };
}