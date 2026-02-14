#include "world.hpp"
#include <algorithm>

uint64_t World::create_entity() {
    uint64_t id = next_entity_id_++;
    entity_info_[id] = {};
    return id;
}

void World::destroy_entity(uint64_t id) {
    auto it = entity_info_.find(id);
    if (it == entity_info_.end()) return;

    ArchetypeId arch_id = it->second.archetype_id;
    if (arch_id != 0) {
        auto arch_it = archetypes_.find(arch_id);
        if (arch_it != archetypes_.end()) {
            auto& arch = arch_it->second;
            auto eit = std::find(arch.entities.begin(), arch.entities.end(), id);
            if (eit != arch.entities.end()) {
                size_t idx = std::distance(arch.entities.begin(), eit);
                size_t count = arch.entity_count();

                // Swap-remove from all columns
                for (auto& col : arch.columns) {
                    col.swap_remove(idx, count);
                }

                // Swap-remove from entity list
                if (idx < count - 1) {
                    arch.entities[idx] = arch.entities[count - 1];
                }
                arch.entities.pop_back();
            }
        }
    }

    entity_info_.erase(it);
}

bool World::entity_exists(uint64_t id) const {
    return entity_info_.count(id) > 0;
}

ArchetypeId World::compute_archetype_id(const std::set<ComponentId>& components) const {
    // Simple hash combining all component IDs
    ArchetypeId hash = 0;
    for (ComponentId cid : components) {
        hash ^= static_cast<ArchetypeId>(cid) * 2654435761ULL;
        hash = (hash << 13) | (hash >> 51);
    }
    return hash == 0 ? 1 : hash;
}

Archetype& World::get_or_create_archetype(ArchetypeId id, const std::set<ComponentId>& components) {
    auto it = archetypes_.find(id);
    if (it != archetypes_.end()) return it->second;

    Archetype arch;
    arch.id = id;
    for (ComponentId cid : components) {
        ComponentArray col;
        col.type = cid;
        col.element_size = component_sizes_[cid];
        arch.columns.push_back(std::move(col));
    }

    archetypes_[id] = std::move(arch);
    return archetypes_[id];
}

void World::migrate_entity(uint64_t entity, ArchetypeId from, ArchetypeId to) {
    auto from_it = archetypes_.find(from);
    auto to_it = archetypes_.find(to);
    if (from_it == archetypes_.end() || to_it == archetypes_.end()) return;

    auto& from_arch = from_it->second;
    auto& to_arch = to_it->second;

    auto eit = std::find(from_arch.entities.begin(), from_arch.entities.end(), entity);
    if (eit == from_arch.entities.end()) return;

    size_t idx = std::distance(from_arch.entities.begin(), eit);

    // Copy matching component data to new archetype
    to_arch.entities.push_back(entity);
    for (auto& to_col : to_arch.columns) {
        auto* from_col = from_arch.get_column(to_col.type);
        if (from_col && idx < from_col->count()) {
            to_col.push_back(from_col->at(idx));
        } else {
            std::vector<uint8_t> zeros(to_col.element_size, 0);
            to_col.push_back(zeros.data());
        }
    }

    // Remove from old archetype
    size_t count = from_arch.entity_count();
    for (auto& col : from_arch.columns) {
        col.swap_remove(idx, count);
    }
    if (idx < count - 1) {
        from_arch.entities[idx] = from_arch.entities[count - 1];
    }
    from_arch.entities.pop_back();
}
