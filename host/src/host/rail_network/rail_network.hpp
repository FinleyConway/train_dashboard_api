#pragma once

#include <vector>
#include <unordered_map>

#include "host/rail_network/track.hpp"
#include "host/rail_network/rail.hpp"
#include "common/api/types.hpp"

namespace host {
    struct rail_location_t {
        size_t track = 0;
        size_t position = 0;
    };

    struct rail_connection_t {
        common::rail_id_t previous = 0; 
        common::rail_id_t next = 0;
        
        common::rail_type_t previous_type = common::rail_type_t::none; 
        common::rail_type_t next_type = common::rail_type_t::none;

        bool branch = false; // dont like this
    };

    enum class rail_status_t {
        success,
        rail_already_exists,
        no_previous_rail,
        invalid_connection,
        invalid_loop
    };

    class rail_network_t {
    public:
        rail_status_t append_rail(const rail_connection_t& connection) {
            // are we starting from a new track?
            if (connection.previous_type == common::rail_type_t::none) {
                return create_track(connection);
            }

            // does the rail reference point exist?
            if (!exists(connection.previous)) {
                return rail_status_t::no_previous_rail;
            }

            // are we evaluating rails on the same track? 
            if (exists(connection.next) && exists(connection.previous))
                return connect_tracks(connection);

            // just add rail to track
            return append_to_track(connection);
        }

        std::optional<common::rail_id_t> get_next_rail(common::rail_id_t current_rail) const {
            if (!exists(current_rail)) return std::nullopt;

            const rail_location_t& location = locate(current_rail);
            const track_t& track = m_tracks[location.track];

            // try next rail on the same track
            if (const rail_t* next_rail = track.get(location.position + 1)) {
                return next_rail->id;
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
                if (const rail_t* first_rail = connected_track.get(0)) {
                    return first_rail->id;
                }

                return std::nullopt;
            }

            // get the branching track
            if (const rail_t* rail = track.get(location.position)) {
                const rail_branch_t& branch = rail->branch;

                if (const rail_t* connected_rail = connected_track.get(branch.position)) {
                    return connected_rail->id;
                }
            }

            return std::nullopt;
        }

        bool exists(common::rail_id_t id) const {
            return m_rail_lookup.contains(id);
        }

        const rail_t* get(common::rail_id_t id) const {
            const rail_location_t& location = locate(id);

            return m_tracks[location.track].get(location.position);
        }

        const rail_t* get(const rail_branch_t& branch) const {
            return m_tracks[branch.track].get(branch.position);
        }

    private:
        const rail_location_t& locate(common::rail_id_t id) const {
            return m_rail_lookup.at(id);
        }

        bool is_junction(common::rail_type_t type) const {
            return (
                type == common::rail_type_t::left_junction ||
                type == common::rail_type_t::right_junction
            );
        }

        rail_status_t create_track(const rail_connection_t& rail) {
            if (m_rail_lookup.contains(rail.next)) {
                return rail_status_t::rail_already_exists;
            }

            // create a new track with an inital track piece
            m_tracks.emplace_back(rail.next, rail.next_type);

            // add rail
            m_rail_lookup.emplace(rail.next, rail_location_t {
                .track = m_tracks.size() - 1,
                .position = 0 // first rail in the newly created track
            });

            return rail_status_t::success;
        }

        rail_status_t append_to_track(const rail_connection_t& connection) {
            if (exists(connection.next)) {
                return rail_status_t::rail_already_exists;
            }

            const rail_location_t& location = locate(connection.previous);

            track_t& track = m_tracks[location.track];
            const size_t index = track.size();
            
            if (connection.branch) {
                return create_branch(location, connection);
            }

            track.add(connection.next, connection.next_type);

            m_rail_lookup.emplace(
                connection.next, 
                rail_location_t {
                    .track = location.track,
                    .position = index
                }
            );

            return rail_status_t::success;
        }

        rail_status_t create_branch(const rail_location_t& location, const rail_connection_t& connection) {
            // can the previous type actually branch?
            if (!is_junction(connection.previous_type)) return rail_status_t::invalid_connection;

            rail_status_t ret = create_track(connection);
            if (ret != rail_status_t::success) {
                return ret;
            }

            m_tracks[location.track].add_branch(
                location.position, 
                rail_branch_t {
                    .track = m_tracks.size() - 1,
                    .position = 0 // first rail in the newly created track
                }
            );
            
            return rail_status_t::success;
        }

        rail_status_t connect_tracks(const rail_connection_t& connection) {
            const rail_location_t& next = locate(connection.next);
            const rail_location_t& previous = locate(connection.previous);

            // it loops
            if (next.track == previous.track) {
                track_t& track = m_tracks[next.track];
                const auto [first, last] = std::minmax(next.position, previous.position);

                if (first != 0 && last != track.size() - 1) {
                    return rail_status_t::invalid_loop;
                }

                // tell to look at the same track
                m_tracks[next.track].set_track_connection(next.track);

                return rail_status_t::success;
            }
            
            // tell to look at another track
            track_t& track = m_tracks[previous.track];
            track.set_track_connection(next.track);
            track.add_branch(previous.position, rail_branch_t {
                .track = next.track,
                .position = next.position
            });

            return rail_status_t::success;
        }

    private:
        std::vector<track_t> m_tracks;
        std::unordered_map<common::rail_id_t, rail_location_t> m_rail_lookup;
    };
}