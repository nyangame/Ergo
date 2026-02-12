#pragma once
#include "shader_graph.hpp"
#include <string>
#include <vector>

// ============================================================
// Optimization pass results
// ============================================================

struct OptimizationResult {
    std::string pass_name;
    uint32_t changes_made = 0;
    std::string description;
};

// ============================================================
// ShaderOptimizer: AI-driven shader optimization
//
// Analyzes a ShaderGraph and generated code to apply:
// - Graph-level: dead node elimination, constant folding, redundancy removal
// - Code-level:  precision hints, instruction reduction, algebraic simplification
// ============================================================

class ShaderOptimizer {
    bool enable_constant_folding_ = true;
    bool enable_dead_code_ = true;
    bool enable_common_subexpr_ = true;
    bool enable_algebraic_ = true;
    bool enable_precision_ = true;
    std::vector<OptimizationResult> report_;

public:
    ShaderOptimizer() = default;

    // Configuration
    void set_constant_folding(bool v) { enable_constant_folding_ = v; }
    void set_dead_code_elimination(bool v) { enable_dead_code_ = v; }
    void set_common_subexpr_elimination(bool v) { enable_common_subexpr_ = v; }
    void set_algebraic_simplification(bool v) { enable_algebraic_ = v; }
    void set_precision_optimization(bool v) { enable_precision_ = v; }

    // --- Graph-level optimization ---
    // Mutates the graph in-place for better code generation
    void optimize_graph(ShaderGraph& graph);

    // --- Code-level optimization ---
    // Post-processes generated shader source
    std::string optimize(const std::string& shader_source) const;

    // Human-readable report of all optimizations applied
    std::string optimization_report() const;

    const std::vector<OptimizationResult>& results() const { return report_; }

private:
    // Graph passes
    uint32_t pass_dead_node_elimination(ShaderGraph& graph);
    uint32_t pass_constant_folding(ShaderGraph& graph);
    uint32_t pass_identity_removal(ShaderGraph& graph);
    uint32_t pass_redundant_cast_removal(ShaderGraph& graph);

    // Code passes
    std::string pass_remove_dead_assignments(const std::string& src) const;
    std::string pass_fold_literal_ops(const std::string& src) const;
    std::string pass_simplify_identity_ops(const std::string& src) const;
    std::string pass_mediump_hints(const std::string& src) const;

    // Helper: evaluate a constant node's float value
    static bool try_eval_constant(const ShaderNode& node, float& out);
};
