#include "shader_optimizer.hpp"
#include <sstream>
#include <regex>
#include <cmath>
#include <algorithm>
#include <unordered_set>

// ============================================================
// Graph-level optimization
// ============================================================

void ShaderOptimizer::optimize_graph(ShaderGraph& graph) {
    report_.clear();

    // Iteratively apply graph passes until no changes
    bool changed = true;
    uint32_t iteration = 0;
    constexpr uint32_t max_iterations = 16;

    while (changed && iteration < max_iterations) {
        changed = false;
        ++iteration;

        if (enable_dead_code_) {
            uint32_t n = pass_dead_node_elimination(graph);
            if (n > 0) {
                report_.push_back({"DeadNodeElimination", n,
                    "Removed " + std::to_string(n) + " unreachable nodes"});
                changed = true;
            }
        }

        if (enable_constant_folding_) {
            uint32_t n = pass_constant_folding(graph);
            if (n > 0) {
                report_.push_back({"ConstantFolding", n,
                    "Folded " + std::to_string(n) + " constant expressions"});
                changed = true;
            }
        }

        if (enable_algebraic_) {
            uint32_t n = pass_identity_removal(graph);
            if (n > 0) {
                report_.push_back({"IdentityRemoval", n,
                    "Removed " + std::to_string(n) + " identity operations (x+0, x*1, etc.)"});
                changed = true;
            }
        }

        if (enable_common_subexpr_) {
            uint32_t n = pass_redundant_cast_removal(graph);
            if (n > 0) {
                report_.push_back({"RedundantCastRemoval", n,
                    "Removed " + std::to_string(n) + " redundant type casts"});
                changed = true;
            }
        }
    }
}

// Remove nodes not reachable from the output node
uint32_t ShaderOptimizer::pass_dead_node_elimination(ShaderGraph& graph) {
    uint32_t output_id = graph.find_output_node();
    if (output_id == 0) return 0;

    // BFS backward from output to find all reachable nodes
    std::unordered_set<uint32_t> reachable;
    std::vector<uint32_t> queue = {output_id};
    reachable.insert(output_id);

    while (!queue.empty()) {
        uint32_t current = queue.back();
        queue.pop_back();

        auto* node = graph.get_node(current);
        if (!node) continue;

        for (uint32_t i = 0; i < node->inputs.size(); ++i) {
            auto* conn = graph.find_input_connection(current, i);
            if (conn && !reachable.count(conn->source_node)) {
                reachable.insert(conn->source_node);
                queue.push_back(conn->source_node);
            }
        }
    }

    // Remove unreachable nodes
    std::vector<uint32_t> to_remove;
    for (auto& [id, node] : graph.nodes()) {
        if (!reachable.count(id)) {
            to_remove.push_back(id);
        }
    }

    for (uint32_t id : to_remove) {
        graph.remove_node(id);
    }

    return static_cast<uint32_t>(to_remove.size());
}

bool ShaderOptimizer::try_eval_constant(const ShaderNode& node, float& out) {
    if (auto* c = std::get_if<NodeConstant>(&node.data)) {
        if (auto* f = std::get_if<float>(&c->value.data)) {
            out = *f;
            return true;
        }
    }
    if (auto* p = std::get_if<NodePropertyFloat>(&node.data)) {
        // Properties are NOT constant-foldable at compile time
        return false;
    }
    return false;
}

// Fold math operations on two constant inputs into a single constant
uint32_t ShaderOptimizer::pass_constant_folding(ShaderGraph& graph) {
    uint32_t folded = 0;

    // Collect nodes to fold (don't modify during iteration)
    struct FoldCandidate {
        uint32_t node_id;
        float result;
    };
    std::vector<FoldCandidate> candidates;

    for (auto& [id, node] : graph.nodes()) {
        auto* math = std::get_if<NodeMath>(&node.data);
        if (!math) continue;

        // Check if all inputs are constants
        bool all_constant = true;
        std::vector<float> input_values;

        for (uint32_t i = 0; i < node.inputs.size(); ++i) {
            auto* conn = graph.find_input_connection(id, i);
            if (!conn) {
                // Use default value
                if (auto* f = std::get_if<float>(&node.inputs[i].default_value.data)) {
                    input_values.push_back(*f);
                } else {
                    all_constant = false;
                    break;
                }
                continue;
            }
            auto* src = graph.get_node(conn->source_node);
            if (!src) { all_constant = false; break; }

            float val;
            if (try_eval_constant(*src, val)) {
                input_values.push_back(val);
            } else {
                all_constant = false;
                break;
            }
        }

        if (!all_constant || input_values.empty()) continue;

        float result = 0.0f;
        float a = input_values[0];
        float b = input_values.size() > 1 ? input_values[1] : 0.0f;

        switch (math->op) {
        case MathOp::Add: result = a + b; break;
        case MathOp::Subtract: result = a - b; break;
        case MathOp::Multiply: result = a * b; break;
        case MathOp::Divide: result = (std::abs(b) > 0.0001f) ? a / b : 0.0f; break;
        case MathOp::Power: result = std::pow(a, b); break;
        case MathOp::SquareRoot: result = (a >= 0.0f) ? std::sqrt(a) : 0.0f; break;
        case MathOp::Abs: result = std::abs(a); break;
        case MathOp::Min: result = std::min(a, b); break;
        case MathOp::Max: result = std::max(a, b); break;
        case MathOp::Negate: result = -a; break;
        case MathOp::Floor: result = std::floor(a); break;
        case MathOp::Ceil: result = std::ceil(a); break;
        case MathOp::Fract: result = a - std::floor(a); break;
        case MathOp::Clamp: {
            float mn = input_values.size() > 1 ? input_values[1] : 0.0f;
            float mx = input_values.size() > 2 ? input_values[2] : 1.0f;
            result = std::clamp(a, mn, mx);
            break;
        }
        case MathOp::Lerp: {
            float t = input_values.size() > 2 ? input_values[2] : 0.5f;
            result = a + (b - a) * t;
            break;
        }
        default:
            continue;
        }

        candidates.push_back({id, result});
    }

    // Replace folded nodes with constants
    for (auto& [node_id, result] : candidates) {
        auto* node = graph.get_node(node_id);
        if (!node) continue;

        // Find all outgoing connections from this node
        auto out_conns = graph.find_output_connections(node_id, 0);

        // Replace with constant node
        node->data = NodeConstant{ShaderValue::from_float(result), ShaderDataType::Float};
        node->name = "Const(" + std::to_string(result) + ")";
        node->inputs.clear();
        if (!node->outputs.empty()) {
            node->outputs[0].default_value = ShaderValue::from_float(result);
        }

        ++folded;
    }

    return folded;
}

// Remove identity operations: x + 0 -> x, x * 1 -> x, x * 0 -> 0
uint32_t ShaderOptimizer::pass_identity_removal(ShaderGraph& graph) {
    uint32_t removed = 0;

    struct Bypass {
        uint32_t node_id;
        uint32_t pass_through_input; // which input to pass through
        bool replace_with_zero;
    };
    std::vector<Bypass> bypasses;

    for (auto& [id, node] : graph.nodes()) {
        auto* math = std::get_if<NodeMath>(&node.data);
        if (!math) continue;
        if (node.inputs.size() < 2) continue;

        // Check each input for known constant values
        auto check_const_input = [&](uint32_t input_idx) -> std::pair<bool, float> {
            auto* conn = graph.find_input_connection(id, input_idx);
            if (!conn) {
                if (auto* f = std::get_if<float>(&node.inputs[input_idx].default_value.data))
                    return {true, *f};
                return {false, 0.0f};
            }
            auto* src = graph.get_node(conn->source_node);
            if (!src) return {false, 0.0f};
            float val;
            if (try_eval_constant(*src, val)) return {true, val};
            return {false, 0.0f};
        };

        auto [a_const, a_val] = check_const_input(0);
        auto [b_const, b_val] = check_const_input(1);

        switch (math->op) {
        case MathOp::Add:
            if (b_const && b_val == 0.0f) bypasses.push_back({id, 0, false});
            else if (a_const && a_val == 0.0f) bypasses.push_back({id, 1, false});
            break;
        case MathOp::Subtract:
            if (b_const && b_val == 0.0f) bypasses.push_back({id, 0, false});
            break;
        case MathOp::Multiply:
            if (b_const && b_val == 1.0f) bypasses.push_back({id, 0, false});
            else if (a_const && a_val == 1.0f) bypasses.push_back({id, 1, false});
            else if (b_const && b_val == 0.0f) bypasses.push_back({id, 0, true});
            else if (a_const && a_val == 0.0f) bypasses.push_back({id, 0, true});
            break;
        case MathOp::Divide:
            if (b_const && b_val == 1.0f) bypasses.push_back({id, 0, false});
            break;
        case MathOp::Power:
            if (b_const && b_val == 1.0f) bypasses.push_back({id, 0, false});
            break;
        default:
            break;
        }
    }

    for (auto& bp : bypasses) {
        if (bp.replace_with_zero) {
            auto* node = graph.get_node(bp.node_id);
            if (!node) continue;
            node->data = NodeConstant{ShaderValue::from_float(0.0f), ShaderDataType::Float};
            node->name = "Const(0)";
            node->inputs.clear();
            ++removed;
        } else {
            // Rewire: all nodes consuming this node's output should instead
            // consume the pass-through input's source
            auto* conn_in = graph.find_input_connection(bp.node_id, bp.pass_through_input);
            if (!conn_in) continue;

            uint32_t pass_src_node = conn_in->source_node;
            uint32_t pass_src_port = conn_in->source_port;

            // Find all connections that use bp.node_id as source
            // We need to gather them first, then rewire
            std::vector<std::pair<uint32_t, uint32_t>> consumers; // (target_node, target_port)
            for (auto& c : graph.connections()) {
                if (c.source_node == bp.node_id) {
                    consumers.push_back({c.target_node, c.target_port});
                }
            }

            for (auto& [tgt_node, tgt_port] : consumers) {
                graph.connect(pass_src_node, pass_src_port, tgt_node, tgt_port);
            }

            graph.remove_node(bp.node_id);
            ++removed;
        }
    }

    return removed;
}

// Remove redundant split+combine or cast chains
uint32_t ShaderOptimizer::pass_redundant_cast_removal(ShaderGraph& graph) {
    uint32_t removed = 0;

    // Detect Swizzle(.xyzw) which is a no-op
    std::vector<uint32_t> noop_swizzles;
    for (auto& [id, node] : graph.nodes()) {
        auto* sw = std::get_if<NodeSwizzle>(&node.data);
        if (!sw) continue;
        if (sw->count == 4 &&
            sw->components[0] == 'x' && sw->components[1] == 'y' &&
            sw->components[2] == 'z' && sw->components[3] == 'w') {
            noop_swizzles.push_back(id);
        }
    }

    for (uint32_t id : noop_swizzles) {
        auto* conn_in = graph.find_input_connection(id, 0);
        if (!conn_in) continue;

        uint32_t pass_src_node = conn_in->source_node;
        uint32_t pass_src_port = conn_in->source_port;

        std::vector<std::pair<uint32_t, uint32_t>> consumers;
        for (auto& c : graph.connections()) {
            if (c.source_node == id) {
                consumers.push_back({c.target_node, c.target_port});
            }
        }

        for (auto& [tgt_node, tgt_port] : consumers) {
            graph.connect(pass_src_node, pass_src_port, tgt_node, tgt_port);
        }

        graph.remove_node(id);
        ++removed;
    }

    return removed;
}

// ============================================================
// Code-level optimization
// ============================================================

std::string ShaderOptimizer::optimize(const std::string& src) const {
    std::string result = src;

    if (enable_dead_code_) {
        result = pass_remove_dead_assignments(result);
    }
    if (enable_constant_folding_) {
        result = pass_fold_literal_ops(result);
    }
    if (enable_algebraic_) {
        result = pass_simplify_identity_ops(result);
    }
    if (enable_precision_) {
        result = pass_mediump_hints(result);
    }

    return result;
}

// Remove variable assignments that are never referenced again
std::string ShaderOptimizer::pass_remove_dead_assignments(const std::string& src) const {
    std::istringstream stream(src);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    // Find variable declarations (pattern: type varname = expr;)
    std::regex decl_pattern(R"(\s+(float|vec[234]|mat[34]|bool|f32|vec[234]<f32>)\s+(n\d+_p\d+\w*)\s*=)");
    std::vector<bool> keep(lines.size(), true);

    for (size_t i = 0; i < lines.size(); ++i) {
        std::smatch match;
        if (std::regex_search(lines[i], match, decl_pattern)) {
            std::string var = match[2].str();

            // Check if this variable is used in any subsequent line
            bool used = false;
            for (size_t j = i + 1; j < lines.size(); ++j) {
                if (lines[j].find(var) != std::string::npos) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                keep[i] = false;
            }
        }
    }

    std::ostringstream result;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (keep[i]) {
            result << lines[i] << "\n";
        }
    }
    return result.str();
}

// Fold operations on literal values (e.g., "0.0 + X" -> "X")
std::string ShaderOptimizer::pass_fold_literal_ops(const std::string& src) const {
    std::string result = src;

    // Fold vec3(X) * 1.0 -> vec3(X)
    std::regex mul_one(R"((\w+)\s*\*\s*1\.0([^0-9]))");
    result = std::regex_replace(result, mul_one, "$1$2");

    // Fold X + 0.0 -> X
    std::regex add_zero(R"((\w+)\s*\+\s*0\.0([^0-9]))");
    result = std::regex_replace(result, add_zero, "$1$2");

    // Fold 0.0 + X -> X
    std::regex zero_add(R"(0\.0\s*\+\s*(\w+))");
    result = std::regex_replace(result, zero_add, "$1");

    return result;
}

// Simplify known identity patterns
std::string ShaderOptimizer::pass_simplify_identity_ops(const std::string& src) const {
    std::string result = src;

    // normalize(normalize(X)) -> normalize(X)
    std::regex double_normalize(R"(normalize\(normalize\(([^)]+)\)\))");
    result = std::regex_replace(result, double_normalize, "normalize($1)");

    // clamp(X, 0.0, 1.0) -> saturate(X)  (GLSL doesn't have saturate, but this aids readability)
    // Actually keep clamp for GLSL compatibility

    // max(X, 0.0) where X is already clamped - skip complex patterns for now

    return result;
}

// Add mediump precision hints for mobile/WebGPU when possible
std::string ShaderOptimizer::pass_mediump_hints(const std::string& src) const {
    // For GLSL 450 (desktop Vulkan), precision qualifiers are not needed
    // This pass is mainly for ES/mobile targets - keeping it as a no-op
    // to avoid breaking desktop shaders
    return src;
}

// ============================================================
// Report
// ============================================================

std::string ShaderOptimizer::optimization_report() const {
    if (report_.empty()) return "No optimizations applied.\n";

    std::ostringstream ss;
    ss << "=== Shader Optimization Report ===\n";
    uint32_t total = 0;
    for (auto& r : report_) {
        ss << "  [" << r.pass_name << "] " << r.description << "\n";
        total += r.changes_made;
    }
    ss << "  Total changes: " << total << "\n";
    ss << "==================================\n";
    return ss.str();
}
