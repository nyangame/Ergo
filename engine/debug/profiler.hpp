#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

class Profiler {
public:
    void begin(const char* name) {
        stack_.push_back({name, Clock::now()});
    }

    void end() {
        if (stack_.empty()) return;
        auto& top = stack_.back();
        auto elapsed = Clock::now() - top.start;
        float ms = std::chrono::duration<float, std::milli>(elapsed).count();
        results_[top.name] = ms;
        stack_.pop_back();
    }

    float get(const char* name) const {
        auto it = results_.find(name);
        return (it != results_.end()) ? it->second : 0.0f;
    }

    const std::unordered_map<std::string, float>& results() const { return results_; }

    void clear() {
        results_.clear();
    }

private:
    using Clock = std::chrono::high_resolution_clock;

    struct ScopeTimer {
        std::string name;
        Clock::time_point start;
    };

    std::vector<ScopeTimer> stack_;
    std::unordered_map<std::string, float> results_;
};

inline Profiler g_profiler;

struct ScopedProfile {
    ScopedProfile(const char* name) : name_(name) { g_profiler.begin(name); }
    ~ScopedProfile() { g_profiler.end(); }
private:
    const char* name_;
};

#define ERGO_PROFILE_SCOPE(name) ScopedProfile _profile_##__LINE__(name)
