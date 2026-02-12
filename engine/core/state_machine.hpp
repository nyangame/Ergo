#pragma once
#include <variant>
#include <type_traits>

template<typename... States>
class StateMachine {
    using StateVariant = std::variant<std::monostate, States...>;
    StateVariant current_;

public:
    template<typename S, typename... Args>
    void transition(Args&&... args) {
        // Exit current state
        std::visit([](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>) {
                if constexpr (requires { s.exit(); })
                    s.exit();
            }
        }, current_);
        // Construct and enter new state
        current_.template emplace<S>(std::forward<Args>(args)...);
        std::get<S>(current_).enter();
    }

    void update(float dt) {
        std::visit([dt](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>)
                s.update(dt);
        }, current_);
    }

    void draw(auto& ctx) {
        std::visit([&ctx](auto& s) {
            if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>) {
                if constexpr (requires { s.draw(ctx); })
                    s.draw(ctx);
            }
        }, current_);
    }

    template<typename S>
    bool is_state() const {
        return std::holds_alternative<S>(current_);
    }
};
