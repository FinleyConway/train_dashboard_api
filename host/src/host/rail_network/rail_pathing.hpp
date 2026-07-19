#pragma once

#include <array>
#include <queue>
#include <optional>
#include <unordered_set>

#include "host/rail_network/rail_network.hpp"

namespace host {
    class rail_pathing_t {
    public:
        explicit rail_pathing_t(const rail_network_t& network) : m_network(network) {
        }

        std::vector<common::rail_id_t> generate_path(common::rail_id_t start, common::rail_id_t end) const {
            if (!m_network.exists(start) || !m_network.exists(end)) return {};

            std::queue<common::rail_id_t> open;
            std::unordered_set<common::rail_id_t> visited;
            std::unordered_map<common::rail_id_t, common::rail_id_t> parent;

            // init starting rail
            open.emplace(start);
            visited.emplace(start);

            while (!open.empty()) {
                common::rail_id_t current = open.front();
                open.pop();

                // has found the target rail
                if (current == end) {
                    return reconstruct(start, end, parent);
                }

                // evaulate next rail and protential branching rail
                for (auto next_opt : get_connections(current)) {
                    if (!next_opt.has_value()) continue;

                    common::rail_id_t rail = next_opt.value();

                    if (visited.contains(rail)) continue;

                    visited.emplace(rail);
                    parent.emplace(rail, current);
                    open.emplace(rail);
                }
            }

            return {};
        }
    
    private:
        std::array<std::optional<common::rail_id_t>, 2> get_connections(common::rail_id_t current) const {
            // move forward on the rail
            auto next_rail_opt = m_network.get_next_rail(current);

            // there is no rail to consider
            if (!next_rail_opt.has_value()) return {};

            common::rail_id_t next_rail = next_rail_opt.value();

            // does this rail have a branch to explore?
            const rail_t* rail_obj = m_network.get(next_rail);

            if (rail_obj->has_branch()) {
                const rail_t* branch_rail_obj = m_network.get(rail_obj->branch);

                return { next_rail, branch_rail_obj->id };
            }

            return { next_rail, std::nullopt };
        }

        std::vector<common::rail_id_t> reconstruct(common::rail_id_t start, common::rail_id_t end, const auto& parent) const {
            std::vector<common::rail_id_t> path;
            common::rail_id_t current = reconstruct.end;

            while (current != reconstruct.start) {
                path.emplace_back(current);
                current = reconstruct.parent.at(current);
            }

            path.emplace_back(reconstruct.start);

            // i could reverse the array but back() and pop_back() seem better?

            return path;
        }

    private:
        const rail_network_t& m_network;
    };
}