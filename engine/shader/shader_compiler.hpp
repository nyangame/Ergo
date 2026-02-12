#pragma once
#include "shader_graph.hpp"
#include <string>
#include <sstream>
#include <unordered_set>

// ============================================================
// Shader output language
// ============================================================

enum class ShaderLanguage : uint8_t {
    GLSL_450,   // Vulkan GLSL
    WGSL        // WebGPU
};

// ============================================================
// ShaderCompiler: generates shader source from a ShaderGraph
// ============================================================

class ShaderCompiler {
    ShaderLanguage language_ = ShaderLanguage::GLSL_450;

public:
    explicit ShaderCompiler(ShaderLanguage lang = ShaderLanguage::GLSL_450)
        : language_(lang) {}

    // Generate vertex shader source
    std::string generate_vertex(const ShaderGraph& graph) const;

    // Generate fragment shader source
    std::string generate_fragment(const ShaderGraph& graph) const;

    // Generate both shaders as a pair (vertex, fragment)
    std::pair<std::string, std::string> generate(const ShaderGraph& graph) const {
        return {generate_vertex(graph), generate_fragment(graph)};
    }

    ShaderLanguage language() const { return language_; }

private:
    // GLSL code generation
    std::string glsl_header() const;
    std::string glsl_vertex_body() const;
    std::string glsl_fragment_uniforms(const ShaderGraph& graph) const;
    std::string glsl_node_expression(const ShaderGraph& graph, uint32_t node_id,
                                     uint32_t port_index,
                                     std::unordered_set<uint32_t>& emitted,
                                     std::ostringstream& body) const;
    std::string glsl_default_value(const ShaderPort& port) const;
    std::string glsl_type_cast(const std::string& expr,
                               ShaderDataType from, ShaderDataType to) const;

    // WGSL code generation
    std::string wgsl_header() const;
    std::string wgsl_vertex_body() const;
    std::string wgsl_fragment_uniforms(const ShaderGraph& graph) const;
    std::string wgsl_node_expression(const ShaderGraph& graph, uint32_t node_id,
                                     uint32_t port_index,
                                     std::unordered_set<uint32_t>& emitted,
                                     std::ostringstream& body) const;
    std::string wgsl_default_value(const ShaderPort& port) const;
    std::string wgsl_type_name(ShaderDataType dt) const;
    std::string wgsl_type_cast(const std::string& expr,
                               ShaderDataType from, ShaderDataType to) const;
};
