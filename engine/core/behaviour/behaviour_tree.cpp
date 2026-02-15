#include "behaviour_tree.hpp"

BTStatus BTNode::tick(float dt) {
    return std::visit([dt](auto& n) -> BTStatus {
        using N = std::decay_t<decltype(n)>;

        if constexpr (std::is_same_v<N, BTAction>) {
            if (n.tick) return n.tick(dt);
            return BTStatus::Failure;
        }
        else if constexpr (std::is_same_v<N, BTCondition>) {
            if (n.check) return n.check() ? BTStatus::Success : BTStatus::Failure;
            return BTStatus::Failure;
        }
        else if constexpr (std::is_same_v<N, BTWait>) {
            n.elapsed += dt;
            if (n.elapsed >= n.duration) {
                n.elapsed = 0.0f;
                return BTStatus::Success;
            }
            return BTStatus::Running;
        }
        else if constexpr (std::is_same_v<N, BTSequence>) {
            while (n.current_index < n.children.size()) {
                auto status = n.children[n.current_index]->tick(dt);
                if (status == BTStatus::Running) return BTStatus::Running;
                if (status == BTStatus::Failure) {
                    n.current_index = 0;
                    return BTStatus::Failure;
                }
                ++n.current_index;
            }
            n.current_index = 0;
            return BTStatus::Success;
        }
        else if constexpr (std::is_same_v<N, BTSelector>) {
            while (n.current_index < n.children.size()) {
                auto status = n.children[n.current_index]->tick(dt);
                if (status == BTStatus::Running) return BTStatus::Running;
                if (status == BTStatus::Success) {
                    n.current_index = 0;
                    return BTStatus::Success;
                }
                ++n.current_index;
            }
            n.current_index = 0;
            return BTStatus::Failure;
        }
        else if constexpr (std::is_same_v<N, BTRepeater>) {
            if (!n.child) return BTStatus::Failure;
            auto status = n.child->tick(dt);
            if (status == BTStatus::Running) return BTStatus::Running;
            ++n.current_count;
            if (n.max_count > 0 && n.current_count >= n.max_count) {
                n.current_count = 0;
                return BTStatus::Success;
            }
            return BTStatus::Running;
        }
        else if constexpr (std::is_same_v<N, BTInverter>) {
            if (!n.child) return BTStatus::Failure;
            auto status = n.child->tick(dt);
            if (status == BTStatus::Success) return BTStatus::Failure;
            if (status == BTStatus::Failure) return BTStatus::Success;
            return BTStatus::Running;
        }
        else {
            return BTStatus::Failure;
        }
    }, data);
}
