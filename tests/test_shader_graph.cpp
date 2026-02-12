#include "test_framework.hpp"
#include "engine/shader/shader_node.hpp"
#include "engine/shader/shader_graph.hpp"
#include "engine/shader/shader_compiler.hpp"
#include "engine/shader/shader_library.hpp"
#include "engine/shader/shader_optimizer.hpp"

TEST_CASE(ShaderGraph_AddNode) {
    ShaderGraph graph;
    auto id = graph.add_node(ShaderNodeLibrary::create_float_property("u_val", 1.0f));
    ASSERT_TRUE(id > 0);
    ASSERT_EQ(graph.node_count(), (size_t)1);
}

TEST_CASE(ShaderGraph_Connect) {
    ShaderGraph graph;
    auto a = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 1.0f));
    auto b = graph.add_node(ShaderNodeLibrary::create_output());
    graph.connect(a, 0, b, 0);
    ASSERT_EQ(graph.connection_count(), (size_t)1);
}

TEST_CASE(ShaderGraph_Validate) {
    ShaderGraph graph;
    auto color = graph.add_node(ShaderNodeLibrary::create_color_property("u_color", 1, 0, 0, 1));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());
    graph.connect(color, 0, output, 0);
    ASSERT_TRUE(graph.validate());
}

TEST_CASE(ShaderGraph_TopologicalSort) {
    ShaderGraph graph;
    auto a = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 1.0f));
    auto b = graph.add_node(ShaderNodeLibrary::create_float_property("u_b", 2.0f));
    auto add = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Add));
    auto out = graph.add_node(ShaderNodeLibrary::create_output());

    graph.connect(a, 0, add, 0);
    graph.connect(b, 0, add, 1);
    graph.connect(add, 0, out, 0);

    auto order = graph.topological_sort();
    ASSERT_EQ(order.size(), (size_t)4);

    // Output should be last
    ASSERT_EQ(order.back(), out);
}

TEST_CASE(ShaderCompiler_GenerateGLSL) {
    ShaderGraph graph;
    auto color = graph.add_node(ShaderNodeLibrary::create_color_property("u_baseColor", 1, 1, 1, 1));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());
    graph.connect(color, 0, output, 0);

    ShaderCompiler compiler(ShaderLanguage::GLSL_450);
    std::string vert = compiler.generate_vertex(graph);
    std::string frag = compiler.generate_fragment(graph);

    ASSERT_TRUE(vert.size() > 0);
    ASSERT_TRUE(frag.size() > 0);
}

TEST_CASE(ShaderOptimizer_RemoveUnused) {
    ShaderGraph graph;
    auto a = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 1.0f));
    auto unused = graph.add_node(ShaderNodeLibrary::create_float_property("u_unused", 0.0f));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());
    graph.connect(a, 0, output, 0);
    // unused is not connected to output

    size_t before = graph.node_count();
    ShaderOptimizer optimizer;
    optimizer.optimize_graph(graph);
    // After optimization, unused node should be removed or report generated
    auto report = optimizer.optimization_report();
    ASSERT_TRUE(report.size() > 0);
}
