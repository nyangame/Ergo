#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

// Forward declarations
struct ShaderNode;
struct ShaderConnection;
struct ShaderPort;
enum class ShaderDataType : uint8_t;

// ============================================================
// Shader composition concepts (C++20)
// ============================================================

// ShaderNodeLike: any type that behaves as a shader graph node
template<typename T>
concept ShaderNodeLike = requires(const T t) {
    { t.node_id() } -> std::convertible_to<uint32_t>;
    { t.node_name() } -> std::convertible_to<std::string_view>;
    { t.input_count() } -> std::convertible_to<size_t>;
    { t.output_count() } -> std::convertible_to<size_t>;
};

// ShaderGraphLike: any type that holds a graph of shader nodes
template<typename T>
concept ShaderGraphLike = requires(T t, uint32_t node_id) {
    { t.node_count() } -> std::convertible_to<size_t>;
    { t.connection_count() } -> std::convertible_to<size_t>;
    { t.find_output_node() } -> std::convertible_to<uint32_t>;
    { t.validate() } -> std::same_as<bool>;
};

// ShaderCodeGenerator: generates shader source from a graph
template<typename T, typename Graph>
concept ShaderCodeGenerator = requires(T t, const Graph& g) {
    { t.generate_vertex(g) } -> std::convertible_to<std::string>;
    { t.generate_fragment(g) } -> std::convertible_to<std::string>;
};

// ShaderOptimizer: optimizes shader source or graph
template<typename T>
concept ShaderOptimizer = requires(T t, const std::string& src) {
    { t.optimize(src) } -> std::convertible_to<std::string>;
    { t.optimization_report() } -> std::convertible_to<std::string>;
};

// ShaderCompilable: can be compiled to GPU bytecode
template<typename T>
concept ShaderCompilable = requires(T t, const std::string& src) {
    { t.compile_vertex(src) } -> std::same_as<bool>;
    { t.compile_fragment(src) } -> std::same_as<bool>;
    { t.error_log() } -> std::convertible_to<std::string>;
};

// TypeConvertible: checks if a shader data type can be auto-cast
template<typename T>
concept ShaderTypeConvertible = requires(T t, ShaderDataType from, ShaderDataType to) {
    { t.can_convert(from, to) } -> std::same_as<bool>;
    { t.conversion_code(from, to) } -> std::convertible_to<std::string>;
};
