#include "demo_framework.hpp"
#include "engine/shader/shader_node.hpp"
#include "engine/shader/shader_graph.hpp"
#include "engine/shader/shader_compiler.hpp"
#include "engine/shader/shader_library.hpp"
#include "engine/shader/shader_optimizer.hpp"
#include <cstdio>

DEMO(Shader_NodeTypes) {
    auto color_prop = ShaderNodeLibrary::create_float_property("u_brightness", 1.0f);
    std::printf("  Float property: '%s' outputs=%zu\n",
                color_prop.name.c_str(), color_prop.outputs.size());

    auto tex_sample = ShaderNodeLibrary::create_texture_sample("u_albedo");
    std::printf("  Texture sample: '%s' inputs=%zu outputs=%zu\n",
                tex_sample.name.c_str(), tex_sample.inputs.size(), tex_sample.outputs.size());

    auto add_node = ShaderNodeLibrary::create_math(MathOp::Add);
    std::printf("  Math add: '%s'\n", add_node.name.c_str());

    auto multiply = ShaderNodeLibrary::create_math(MathOp::Multiply);
    std::printf("  Math multiply: '%s'\n", multiply.name.c_str());
}

DEMO(Shader_Graph_Build) {
    ShaderGraph graph;

    auto color_id = graph.add_node(ShaderNodeLibrary::create_color_property("u_color", 1, 0, 0, 1));
    auto brightness_id = graph.add_node(ShaderNodeLibrary::create_float_property("u_brightness", 0.8f));
    auto mul_id = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Multiply));
    auto output_id = graph.add_node(ShaderNodeLibrary::create_output());

    std::printf("  Nodes: %zu\n", graph.node_count());

    graph.connect(color_id, 0, mul_id, 0);
    graph.connect(brightness_id, 0, mul_id, 1);
    graph.connect(mul_id, 0, output_id, 0);

    std::printf("  Connections: %zu\n", graph.connection_count());

    bool valid = graph.validate();
    std::printf("  Graph valid: %s\n", valid ? "yes" : "no");

    auto order = graph.topological_sort();
    std::printf("  Topological order: ");
    for (auto id : order) std::printf("%u ", id);
    std::printf("\n");
}

DEMO(Shader_Compile) {
    ShaderGraph graph;

    auto color_id = graph.add_node(ShaderNodeLibrary::create_color_property("u_baseColor", 1, 1, 1, 1));
    auto output_id = graph.add_node(ShaderNodeLibrary::create_output());
    graph.connect(color_id, 0, output_id, 0);

    ShaderCompiler compiler(ShaderLanguage::GLSL_450);

    std::string vert = compiler.generate_vertex(graph);
    std::string frag = compiler.generate_fragment(graph);
    std::printf("  GLSL vertex shader: %zu chars\n", vert.size());
    std::printf("  GLSL fragment shader: %zu chars\n", frag.size());

    if (!frag.empty()) {
        // Print first few lines
        std::printf("  Fragment preview:\n");
        size_t pos = 0;
        int lines = 0;
        while (pos < frag.size() && lines < 5) {
            size_t nl = frag.find('\n', pos);
            if (nl == std::string::npos) nl = frag.size();
            std::printf("    %.*s\n", (int)(nl - pos), frag.c_str() + pos);
            pos = nl + 1;
            ++lines;
        }
        if (pos < frag.size()) std::printf("    ...\n");
    }
}

DEMO(Shader_Optimizer) {
    ShaderGraph graph;

    auto c1 = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 2.0f));
    auto c2 = graph.add_node(ShaderNodeLibrary::create_float_property("u_b", 3.0f));
    auto add = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Add));
    auto unused = graph.add_node(ShaderNodeLibrary::create_float_property("u_unused", 0.0f));
    auto output = graph.add_node(ShaderNodeLibrary::create_output());

    graph.connect(c1, 0, add, 0);
    graph.connect(c2, 0, add, 1);
    graph.connect(add, 0, output, 0);
    // unused node is not connected to output

    std::printf("  Before optimization: %zu nodes\n", graph.node_count());

    ShaderOptimizer optimizer;
    optimizer.optimize_graph(graph);
    auto report = optimizer.optimization_report();
    std::printf("  Optimization report: %s\n", report.c_str());
}
