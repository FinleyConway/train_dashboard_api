#pragma once

#include <vector>
#include <unordered_map>

#include "host/rail_network/track.hpp"
#include "host/rail_network/rail.hpp"
#include "common/api/types.hpp"

namespace host {
    struct rail_lookup_t {
        size_t index = 0;
        size_t track_index = 0;
    };

    class rail_network_t {
    public:
        void append_rail(common::rail_id_t id, common::rail_type_t type, common::rail_id_t from_id, common::rail_type_t from_type, bool branch_from) {
            // are we starting from a new track?
            if (from_type == common::rail_type_t::none) {
                init_track(id, type, from_type);
            }
            else {
                if (m_track_lookup.contains(id) && m_track_lookup.contains(from_id)) {
                    connect_track(id, type, from_id, from_type);
                }
                else {
                    add_rail_to_track(id, type, from_id, from_type, branch_from);
                }
            }
        }

        void get_next_rail(size_t offset) const {

        }

        uint32_t get_distance() const {

        }

    private:
        void init_track(common::rail_id_t id, common::rail_type_t type, common::rail_type_t from_type) {
            // create a new track with an inital track piece
            m_tracks.emplace_back(type);

            // add rail
            m_track_lookup.emplace(id, rail_lookup_t {
                .index = 0, // first rail in the newly created track
                .track_index = m_tracks.size() - 1
            });
        }

        void add_rail_to_track(common::rail_id_t id, common::rail_type_t type, common::rail_id_t from_id, common::rail_type_t from_type, bool branch_from) {
            // find where the rail is
            const rail_lookup_t& lookup = m_track_lookup.at(from_id);
            
            if (branch_from) {
                create_rail_branch(lookup, id, type, from_type);
            }
            else {
                // add rail to track
                m_tracks.at(lookup.track_index).add(type);
            }
        }

        void create_rail_branch(const rail_lookup_t& lookup, common::rail_id_t id, common::rail_type_t type, common::rail_type_t from_type) {
            // can the from type actually branch?
            if (from_type != common::rail_type_t::left_junction || from_type != common::rail_type_t::right_junction) return;

            init_track(id, type, from_type);

            // tell the previous track that it has a branching track
            rail_t& rail = m_tracks.at(lookup.track_index).get(lookup.index);

            rail.branch = rail_branch_t {
                .index = 0, // first rail in the newly created track
                .track_index = m_tracks.size() - 1
            };
        }

        void connect_track(common::rail_id_t id, common::rail_type_t type, common::rail_id_t from_id, common::rail_type_t from_type) {
            const rail_lookup_t& to_lookup = m_track_lookup.at(id);
            const rail_lookup_t& from_lookup = m_track_lookup.at(from_id);

            const size_t to_track = to_lookup.track_index;
            const size_t from_track = from_lookup.track_index;

            // it loops
            if (to_track == from_track) {
                // tell to look at the same track
                m_tracks.at(to_track).set_track_connection(to_track);
            }
            else {
                // tell to look at another track
                m_tracks.at(from_track).set_track_connection(to_track);
            }
        }

    private:
        std::vector<track_t> m_tracks;
        std::unordered_map<common::rail_id_t, rail_lookup_t> m_track_lookup;
    };
}