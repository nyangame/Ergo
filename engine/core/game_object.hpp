#pragma once
#include <string>
#include <cstdint>
#include <any>
#include <unordered_map>
#include <typeindex>
#include "../math/transform.hpp"

struct GameObject {
    uint64_t id = 0;
    std::string name_;
    uint32_t object_type_ = 0;
    Transform2D transform_;
    std::unordered_map<std::type_index, std::any> components_;

    // Satisfies GameObjectLike concept
    Transform2D& transform() { return transform_; }
    std::string_view name() const { return name_; }
    uint32_t object_type() const { return object_type_; }

    template<typename T>
    void add_component(T&& comp) {
        components_[std::type_index(typeid(std::decay_t<T>))] = std::forward<T>(comp);
    }

    template<typename T>
    T* get_component() {
        auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }

    template<typename T>
    const T* get_component() const {
        auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) return nullptr;
        return std::any_cast<T>(&it->second);
    }
};
