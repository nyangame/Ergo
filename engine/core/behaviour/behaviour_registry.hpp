#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "behaviour.hpp"

// ============================================================
// BehaviourRegistry: factory registry for GUI-driven composition
//   GUI enumerates registered types and adds them to objects
// ============================================================

class BehaviourRegistry {
public:
    struct Entry {
        std::string name;
        std::string category;
        ThreadingPolicy policy = ThreadingPolicy::MainThread;
        bool thread_aware = false;
        std::function<std::unique_ptr<IBehaviour>()> factory;
    };

    template<BehaviourLike T>
    void register_type(std::string_view category = "General") {
        Entry entry;
        entry.name = std::string(T::type_name());
        entry.category = std::string(category);
        if constexpr (ThreadAware<T>) {
            entry.policy = T::threading_policy();
            entry.thread_aware = true;
        }
        entry.factory = []() -> std::unique_ptr<IBehaviour> {
            return std::make_unique<BehaviourModel<T>>();
        };
        entries_[entry.name] = std::move(entry);
    }

    std::unique_ptr<IBehaviour> create(std::string_view name) const {
        auto it = entries_.find(std::string(name));
        if (it == entries_.end()) return nullptr;
        return it->second.factory();
    }

    std::vector<std::string_view> names() const {
        std::vector<std::string_view> result;
        result.reserve(entries_.size());
        for (auto& [k, _] : entries_)
            result.push_back(k);
        return result;
    }

    std::vector<std::string_view> names_in_category(std::string_view cat) const {
        std::vector<std::string_view> result;
        for (auto& [k, e] : entries_) {
            if (e.category == cat)
                result.push_back(k);
        }
        return result;
    }

    std::vector<std::string> categories() const {
        std::vector<std::string> result;
        for (auto& [_, e] : entries_) {
            bool found = false;
            for (auto& c : result) {
                if (c == e.category) { found = true; break; }
            }
            if (!found) result.push_back(e.category);
        }
        return result;
    }

    const Entry* find(std::string_view name) const {
        auto it = entries_.find(std::string(name));
        if (it == entries_.end()) return nullptr;
        return &it->second;
    }

    size_t size() const { return entries_.size(); }

private:
    std::unordered_map<std::string, Entry> entries_;
};

// Global registry (follows g_physics / g_time pattern)
inline BehaviourRegistry g_behaviour_registry;
