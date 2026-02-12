#pragma once
#include <cstdint>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <cstring>

using ComponentId = uint32_t;
using ArchetypeId = uint64_t;

struct ComponentArray {
    ComponentId type = 0;
    size_t element_size = 0;
    std::vector<uint8_t> data;

    void* at(size_t index) {
        return data.data() + index * element_size;
    }

    const void* at(size_t index) const {
        return data.data() + index * element_size;
    }

    void push_back(const void* element) {
        size_t old_size = data.size();
        data.resize(old_size + element_size);
        std::memcpy(data.data() + old_size, element, element_size);
    }

    void swap_remove(size_t index, size_t count) {
        if (index >= count || count == 0) return;
        if (index < count - 1) {
            std::memcpy(at(index), at(count - 1), element_size);
        }
        data.resize((count - 1) * element_size);
    }

    size_t count() const {
        return element_size > 0 ? data.size() / element_size : 0;
    }
};

struct Archetype {
    ArchetypeId id = 0;
    std::vector<ComponentArray> columns;
    std::vector<uint64_t> entities;  // entity IDs stored in this archetype

    size_t entity_count() const { return entities.size(); }

    ComponentArray* get_column(ComponentId type) {
        for (auto& col : columns) {
            if (col.type == type) return &col;
        }
        return nullptr;
    }

    const ComponentArray* get_column(ComponentId type) const {
        for (auto& col : columns) {
            if (col.type == type) return &col;
        }
        return nullptr;
    }
};
