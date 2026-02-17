#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <variant>
#include "behaviour.hpp"

// ============================================================
// BehaviourTree: variant-based AI decision tree
//   Node types are value types stored in std::variant
//   GUI assembles tree structure by adding/nesting nodes
// ============================================================

// Result of a node tick
enum class BTStatus {
    Success,
    Failure,
    Running
};

// Forward declaration
struct BTNode;

// ============================================================
// Leaf nodes
// ============================================================

// Action: executes a user-defined function
struct BTAction {
    std::string name;
    std::function<BTStatus(float dt)> tick;
};

// Condition: checks a predicate
struct BTCondition {
    std::string name;
    std::function<bool()> check;
};

// Wait: pauses for a given duration
struct BTWait {
    float duration = 1.0f;
    float elapsed = 0.0f;
};

// ============================================================
// Composite nodes
// ============================================================

// Sequence: runs children left-to-right, fails on first failure
struct BTSequence {
    std::string name;
    std::vector<std::unique_ptr<BTNode>> children;
    size_t current_index = 0;
};

// Selector: runs children left-to-right, succeeds on first success
struct BTSelector {
    std::string name;
    std::vector<std::unique_ptr<BTNode>> children;
    size_t current_index = 0;
};

// Repeater: repeats child N times (0 = infinite)
struct BTRepeater {
    uint32_t max_count = 0;
    uint32_t current_count = 0;
    std::unique_ptr<BTNode> child;
};

// ============================================================
// Decorator nodes
// ============================================================

// Inverter: inverts child result
struct BTInverter {
    std::unique_ptr<BTNode> child;
};

// ============================================================
// BTNode: variant holding any node type
// ============================================================

struct BTNode {
    using Variant = std::variant<
        BTAction,
        BTCondition,
        BTWait,
        BTSequence,
        BTSelector,
        BTRepeater,
        BTInverter
    >;

    Variant data;
    std::string label;

    // Tick dispatched via std::visit
    BTStatus tick(float dt);

    // Factory helpers for GUI-driven construction
    static std::unique_ptr<BTNode> make_action(std::string name,
                                                std::function<BTStatus(float)> fn) {
        auto node = std::make_unique<BTNode>();
        node->label = name;
        node->data = BTAction{std::move(name), std::move(fn)};
        return node;
    }

    static std::unique_ptr<BTNode> make_condition(std::string name,
                                                   std::function<bool()> fn) {
        auto node = std::make_unique<BTNode>();
        node->label = name;
        node->data = BTCondition{std::move(name), std::move(fn)};
        return node;
    }

    static std::unique_ptr<BTNode> make_wait(float seconds) {
        auto node = std::make_unique<BTNode>();
        node->label = "Wait";
        node->data = BTWait{seconds, 0.0f};
        return node;
    }

    static std::unique_ptr<BTNode> make_sequence(std::string name) {
        auto node = std::make_unique<BTNode>();
        node->label = name;
        node->data = BTSequence{std::move(name), {}, 0};
        return node;
    }

    static std::unique_ptr<BTNode> make_selector(std::string name) {
        auto node = std::make_unique<BTNode>();
        node->label = name;
        node->data = BTSelector{std::move(name), {}, 0};
        return node;
    }

    static std::unique_ptr<BTNode> make_repeater(uint32_t count,
                                                  std::unique_ptr<BTNode> child) {
        auto node = std::make_unique<BTNode>();
        node->label = "Repeater";
        node->data = BTRepeater{count, 0, std::move(child)};
        return node;
    }

    static std::unique_ptr<BTNode> make_inverter(std::unique_ptr<BTNode> child) {
        auto node = std::make_unique<BTNode>();
        node->label = "Inverter";
        node->data = BTInverter{std::move(child)};
        return node;
    }

    // Add child to composite node (Sequence/Selector)
    void add_child(std::unique_ptr<BTNode> child) {
        std::visit([&](auto& n) {
            using N = std::decay_t<decltype(n)>;
            if constexpr (std::is_same_v<N, BTSequence> ||
                          std::is_same_v<N, BTSelector>) {
                n.children.push_back(std::move(child));
            }
        }, data);
    }
};

// ============================================================
// BehaviourTree: root behaviour that ticks a BTNode tree
// ============================================================

struct BehaviourTree {
    std::unique_ptr<BTNode> root;
    BTStatus last_status = BTStatus::Failure;

    static constexpr std::string_view type_name() { return "BehaviourTree"; }
    static constexpr ThreadingPolicy threading_policy() { return ThreadingPolicy::AnyThread; }

    void start() {
        last_status = BTStatus::Failure;
    }

    void update(float dt) {
        if (root) {
            last_status = root->tick(dt);
        }
    }

    void release() {
        root.reset();
    }
};
