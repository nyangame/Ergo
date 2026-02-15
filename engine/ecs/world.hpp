#pragma once
#include "archetype.hpp"
#include "../core/job_system.hpp"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <set>
#include <algorithm>
#include <array>
#include <utility>

class World {
public:
    uint64_t create_entity();
    void destroy_entity(uint64_t id);
    bool entity_exists(uint64_t id) const;

    template<typename T>
    void add_component(uint64_t entity, T component) {
        ComponentId cid = component_id<T>();
        auto& info = entity_info_[entity];
        info.components.insert(cid);

        ArchetypeId arch_id = compute_archetype_id(info.components);
        auto& arch = get_or_create_archetype(arch_id, info.components);

        // If entity was in a different archetype, migrate
        if (info.archetype_id != 0 && info.archetype_id != arch_id) {
            migrate_entity(entity, info.archetype_id, arch_id);
        }

        info.archetype_id = arch_id;

        // Find or add entity to archetype
        auto it = std::find(arch.entities.begin(), arch.entities.end(), entity);
        if (it == arch.entities.end()) {
            arch.entities.push_back(entity);
            // Add default values for all columns
            for (auto& col : arch.columns) {
                if (col.type == cid) {
                    col.push_back(&component);
                } else {
                    std::vector<uint8_t> zeros(col.element_size, 0);
                    col.push_back(zeros.data());
                }
            }
        } else {
            // Update existing component
            size_t idx = std::distance(arch.entities.begin(), it);
            auto* col = arch.get_column(cid);
            if (col) {
                std::memcpy(col->at(idx), &component, sizeof(T));
            }
        }
    }

    template<typename T>
    T* get_component(uint64_t entity) {
        auto it = entity_info_.find(entity);
        if (it == entity_info_.end()) return nullptr;

        ComponentId cid = component_id<T>();
        auto arch_it = archetypes_.find(it->second.archetype_id);
        if (arch_it == archetypes_.end()) return nullptr;

        auto& arch = arch_it->second;
        auto eit = std::find(arch.entities.begin(), arch.entities.end(), entity);
        if (eit == arch.entities.end()) return nullptr;

        size_t idx = std::distance(arch.entities.begin(), eit);
        auto* col = arch.get_column(cid);
        if (!col) return nullptr;

        return reinterpret_cast<T*>(col->at(idx));
    }

    template<typename T>
    bool has_component(uint64_t entity) const {
        auto it = entity_info_.find(entity);
        if (it == entity_info_.end()) return false;
        return it->second.components.count(component_id<T>()) > 0;
    }

    // Query: iterate entities with specific components
    template<typename... Ts, typename Func>
    void each(Func&& fn) {
        std::set<ComponentId> required = {component_id<Ts>()...};

        for (auto& [arch_id, arch] : archetypes_) {
            // Check if archetype has all required components
            bool has_all = true;
            for (ComponentId cid : required) {
                if (!arch.get_column(cid)) { has_all = false; break; }
            }
            if (!has_all) continue;

            for (size_t i = 0; i < arch.entity_count(); ++i) {
                fn(arch.entities[i],
                   *reinterpret_cast<Ts*>(arch.get_column(component_id<Ts>())->at(i))...);
            }
        }
    }

    // Parallel query: iterate entities using the global JobSystem.
    // Each archetype's entities are split into chunks and processed
    // on worker threads. The callback receives (entity_id, components...).
    // chunk_size should be tuned for cache line alignment (e.g., 64 entities
    // keeps ~4KB per component column chunk in L1 for 64-byte components).
    template<typename... Ts, typename Func>
    void parallel_each(Func&& fn, uint32_t chunk_size = 64) {
        parallel_each_impl<Ts...>(std::index_sequence_for<Ts...>{},
                                   std::forward<Func>(fn), chunk_size);
    }

private:
    template<typename... Ts, size_t... Is, typename Func>
    void parallel_each_impl(std::index_sequence<Is...>, Func&& fn, uint32_t chunk_size) {
        std::set<ComponentId> required = {component_id<Ts>()...};

        for (auto& [arch_id, arch] : archetypes_) {
            bool has_all = true;
            for (ComponentId cid : required) {
                if (!arch.get_column(cid)) { has_all = false; break; }
            }
            if (!has_all) continue;

            uint32_t count = static_cast<uint32_t>(arch.entity_count());
            if (count == 0) continue;

            // Cache raw byte pointers per component column.
            // Column data is contiguous (SOA layout) so sequential
            // access within a chunk is cache-line friendly.
            std::array<uint8_t*, sizeof...(Ts)> col_data = {
                arch.get_column(component_id<Ts>())->data.data()...
            };

            const uint64_t* entity_ids = arch.entities.data();

            g_job_system.parallel_for(0, count, chunk_size,
                [entity_ids, &fn, col_data](uint32_t begin, uint32_t end) {
                    for (uint32_t i = begin; i < end; ++i) {
                        fn(entity_ids[i],
                           *reinterpret_cast<Ts*>(col_data[Is] + i * sizeof(Ts))...);
                    }
                });
        }
    }

public:

    size_t entity_count() const { return entity_info_.size(); }
    size_t archetype_count() const { return archetypes_.size(); }

private:
    struct EntityInfo {
        ArchetypeId archetype_id = 0;
        std::set<ComponentId> components;
    };

    uint64_t next_entity_id_ = 1;
    std::unordered_map<uint64_t, EntityInfo> entity_info_;
    std::unordered_map<ArchetypeId, Archetype> archetypes_;

    static inline std::unordered_map<std::type_index, ComponentId> type_to_id_;
    static inline ComponentId next_component_id_ = 1;
    static inline std::unordered_map<ComponentId, size_t> component_sizes_;

    template<typename T>
    static ComponentId component_id() {
        auto idx = std::type_index(typeid(T));
        auto it = type_to_id_.find(idx);
        if (it != type_to_id_.end()) return it->second;
        ComponentId id = next_component_id_++;
        type_to_id_[idx] = id;
        component_sizes_[id] = sizeof(T);
        return id;
    }

    ArchetypeId compute_archetype_id(const std::set<ComponentId>& components) const;
    Archetype& get_or_create_archetype(ArchetypeId id, const std::set<ComponentId>& components);
    void migrate_entity(uint64_t entity, ArchetypeId from, ArchetypeId to);
};
