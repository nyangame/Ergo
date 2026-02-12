#pragma once
#include "shader_graph.hpp"
#include "shader_compiler.hpp"
#include "shader_optimizer.hpp"
#include "../render/mesh.hpp"
#include <string>
#include <unordered_map>
#include <optional>

// ============================================================
// ShaderMaterial: a material driven by a shader composition graph
//
// Wraps a ShaderGraph, compiles it, optimizes it, and holds
// the generated shader source ready for the renderer backend.
// ============================================================

class ShaderMaterial {
    ShaderGraph graph_;
    ShaderCompiler compiler_;
    ShaderOptimizer optimizer_;

    // Cached compiled sources
    std::string vertex_source_;
    std::string fragment_source_;
    bool compiled_ = false;
    bool optimized_ = false;

    // Runtime uniform values
    std::unordered_map<std::string, float> float_uniforms_;
    std::unordered_map<std::string, std::array<float, 4>> vec4_uniforms_;
    std::unordered_map<std::string, uint64_t> texture_bindings_;

    // Links back to render pipeline
    uint64_t material_id_ = 0;
    uint64_t gpu_pipeline_ = 0;

public:
    explicit ShaderMaterial(ShaderLanguage lang = ShaderLanguage::GLSL_450)
        : compiler_(lang) {}

    ShaderMaterial(ShaderGraph graph, ShaderLanguage lang = ShaderLanguage::GLSL_450)
        : graph_(std::move(graph)), compiler_(lang) {}

    // --- Graph access ---
    ShaderGraph& graph() { return graph_; }
    const ShaderGraph& graph() const { return graph_; }

    // --- Compile the graph into shader source ---
    bool compile() {
        if (!graph_.validate()) return false;

        // Optimize graph first
        optimizer_.optimize_graph(graph_);
        optimized_ = true;

        // Generate source
        auto [vert, frag] = compiler_.generate(graph_);

        // Optimize generated code
        vertex_source_ = optimizer_.optimize(vert);
        fragment_source_ = optimizer_.optimize(frag);

        compiled_ = true;
        return true;
    }

    bool is_compiled() const { return compiled_; }
    bool is_optimized() const { return optimized_; }

    const std::string& vertex_source() const { return vertex_source_; }
    const std::string& fragment_source() const { return fragment_source_; }

    std::string optimization_report() const { return optimizer_.optimization_report(); }

    // --- Uniform setters ---
    void set_float(const std::string& name, float value) {
        float_uniforms_[name] = value;
    }

    void set_vec4(const std::string& name, float x, float y, float z, float w) {
        vec4_uniforms_[name] = {x, y, z, w};
    }

    void set_texture(const std::string& name, uint64_t texture_handle) {
        texture_bindings_[name] = texture_handle;
    }

    // --- Uniform getters ---
    std::optional<float> get_float(const std::string& name) const {
        auto it = float_uniforms_.find(name);
        if (it == float_uniforms_.end()) return std::nullopt;
        return it->second;
    }

    const std::unordered_map<std::string, float>& float_uniforms() const {
        return float_uniforms_;
    }
    const std::unordered_map<std::string, std::array<float, 4>>& vec4_uniforms() const {
        return vec4_uniforms_;
    }
    const std::unordered_map<std::string, uint64_t>& texture_bindings() const {
        return texture_bindings_;
    }

    // --- GPU pipeline handle (set by renderer after creating GPU pipeline) ---
    void set_material_id(uint64_t id) { material_id_ = id; }
    uint64_t material_id() const { return material_id_; }
    void set_gpu_pipeline(uint64_t handle) { gpu_pipeline_ = handle; }
    uint64_t gpu_pipeline() const { return gpu_pipeline_; }

    // --- Create a MaterialData compatible with existing render pipeline ---
    MaterialData to_material_data() const {
        MaterialData mat;
        mat.id = material_id_;
        mat.name = graph_.name();

        // Map common properties
        if (auto it = vec4_uniforms_.find("base_color"); it != vec4_uniforms_.end()) {
            auto& c = it->second;
            mat.diffuse_color = Color{
                static_cast<uint8_t>(c[0] * 255.0f),
                static_cast<uint8_t>(c[1] * 255.0f),
                static_cast<uint8_t>(c[2] * 255.0f),
                static_cast<uint8_t>(c[3] * 255.0f)
            };
        }
        if (auto it = float_uniforms_.find("metallic"); it != float_uniforms_.end()) {
            mat.metallic = it->second;
        }
        if (auto it = float_uniforms_.find("roughness"); it != float_uniforms_.end()) {
            mat.roughness = it->second;
        }
        if (auto it = texture_bindings_.find("diffuse_map"); it != texture_bindings_.end()) {
            mat.diffuse_texture = it->second;
        }
        if (auto it = texture_bindings_.find("normal_map"); it != texture_bindings_.end()) {
            mat.normal_texture = it->second;
        }

        return mat;
    }
};

// ============================================================
// Convenience: build common material presets
// ============================================================

namespace shader_presets {

// PBR material with texture inputs
inline ShaderMaterial create_pbr() {
    ShaderGraph graph("PBR");

    auto tex_albedo = graph.add_node(ShaderNodeLibrary::create_texture_sample("diffuse_map"));
    auto tex_normal = graph.add_node(ShaderNodeLibrary::create_texture_sample("normal_map"));
    auto prop_metallic = graph.add_node(ShaderNodeLibrary::create_float_property("metallic", 0.0f));
    auto prop_roughness = graph.add_node(ShaderNodeLibrary::create_float_property("roughness", 0.5f));
    auto normal = graph.add_node(ShaderNodeLibrary::create_normal());
    auto lighting = graph.add_node(ShaderNodeLibrary::create_lighting(LightModel::CookTorrance));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());

    // Albedo texture -> Lighting albedo
    graph.connect(tex_albedo, 1, lighting, 1);  // RGB -> Albedo
    // World normal -> Lighting normal
    graph.connect(normal, 0, lighting, 0);
    // Properties -> Lighting
    graph.connect(prop_metallic, 0, lighting, 3);
    graph.connect(prop_roughness, 0, lighting, 4);
    // Lighting output -> Surface albedo
    graph.connect(lighting, 0, output, 0);    // Color -> Albedo
    // Metallic/Roughness -> Surface
    graph.connect(prop_metallic, 0, output, 2);
    graph.connect(prop_roughness, 0, output, 3);
    // Texture alpha -> Surface alpha
    graph.connect(tex_albedo, 5, output, 5);  // A -> Alpha

    return ShaderMaterial(std::move(graph));
}

// Simple unlit material
inline ShaderMaterial create_unlit() {
    ShaderGraph graph("Unlit");

    auto color = graph.add_node(ShaderNodeLibrary::create_color_property("base_color", 1, 1, 1, 1));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());

    graph.connect(color, 1, output, 0);  // RGB -> Albedo
    graph.connect(color, 5, output, 5);  // A -> Alpha

    return ShaderMaterial(std::move(graph));
}

// Toon shading material
inline ShaderMaterial create_toon() {
    ShaderGraph graph("Toon");

    auto color = graph.add_node(ShaderNodeLibrary::create_color_property("base_color", 1, 0.5, 0.2, 1));
    auto normal = graph.add_node(ShaderNodeLibrary::create_normal());
    auto view = graph.add_node(ShaderNodeLibrary::create_view_direction());
    auto lighting = graph.add_node(ShaderNodeLibrary::create_lighting(LightModel::Toon));
    auto fresnel = graph.add_node(ShaderNodeLibrary::create_fresnel(3.0f));
    auto outline_color = graph.add_node(ShaderNodeLibrary::create_constant_vec3(0.0f, 0.0f, 0.0f));
    auto blend = graph.add_node(ShaderNodeLibrary::create_blend(BlendMode::Normal));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());

    // Normal -> Lighting, Fresnel
    graph.connect(normal, 0, lighting, 0);
    graph.connect(normal, 0, fresnel, 0);
    graph.connect(view, 0, fresnel, 1);
    // Color -> Lighting albedo
    graph.connect(color, 1, lighting, 1);
    // Blend lighting with outline
    graph.connect(lighting, 0, blend, 0);     // Base = lit color
    graph.connect(outline_color, 0, blend, 1); // Blend = outline color
    graph.connect(fresnel, 0, blend, 2);       // Opacity = fresnel (edge)
    // Output
    graph.connect(blend, 0, output, 0);

    return ShaderMaterial(std::move(graph));
}

// Animated dissolve effect
inline ShaderMaterial create_dissolve() {
    ShaderGraph graph("Dissolve");

    auto tex_albedo = graph.add_node(ShaderNodeLibrary::create_texture_sample("diffuse_map"));
    auto tex_noise = graph.add_node(ShaderNodeLibrary::create_texture_sample("noise_map"));
    auto prop_threshold = graph.add_node(ShaderNodeLibrary::create_float_property("threshold", 0.5f));
    auto prop_edge_width = graph.add_node(ShaderNodeLibrary::create_float_property("edge_width", 0.05f));
    auto edge_color = graph.add_node(ShaderNodeLibrary::create_color_property("edge_color", 1, 0.5, 0, 1));

    auto compare = graph.add_node(ShaderNodeLibrary::create_compare(CompareOp::Greater));
    auto sub = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Subtract));
    auto abs_node = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Abs));
    auto step_node = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Step));
    auto blend = graph.add_node(ShaderNodeLibrary::create_blend(BlendMode::Additive));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());

    // Noise R channel vs threshold -> alpha clip
    graph.connect(tex_noise, 2, compare, 0);     // Noise R -> A
    graph.connect(prop_threshold, 0, compare, 1); // Threshold -> B

    // Edge glow: abs(noise - threshold) < edge_width
    graph.connect(tex_noise, 2, sub, 0);
    graph.connect(prop_threshold, 0, sub, 1);
    graph.connect(sub, 0, abs_node, 0);
    graph.connect(prop_edge_width, 0, step_node, 0); // Edge -> edge
    graph.connect(abs_node, 0, step_node, 1);        // |noise - threshold| -> X

    // Blend albedo with edge color
    graph.connect(tex_albedo, 1, blend, 0);
    graph.connect(edge_color, 1, blend, 1);
    graph.connect(step_node, 0, blend, 2);

    // Output
    graph.connect(blend, 0, output, 0);     // Blended color -> Albedo
    graph.connect(edge_color, 1, output, 4); // Edge -> Emission

    return ShaderMaterial(std::move(graph));
}

} // namespace shader_presets
