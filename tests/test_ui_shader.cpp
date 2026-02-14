#include "framework/test_framework.hpp"
#include "engine/ui/ui_element.hpp"
#include "engine/ui/ui_widgets.hpp"
#include "engine/shader/shader_node.hpp"
#include "engine/shader/shader_graph.hpp"
#include "engine/shader/shader_compiler.hpp"
#include "engine/shader/shader_library.hpp"
#include "engine/shader/shader_optimizer.hpp"

using namespace ergo::test;

// ---------------------------------------------------------------------------
// UI/Element suite
// ---------------------------------------------------------------------------
static TestSuite ui_element("UI/Element");

static auto ui_init = []() {
    ui_element.add("UIElement_Contains", [](TestContext& ctx) {
        UIElement elem;
        elem.position = {100.0f, 100.0f};
        elem.size = {200.0f, 150.0f};

        ERGO_TEST_ASSERT_TRUE(ctx, elem.contains({150.0f, 150.0f}));
        ERGO_TEST_ASSERT_TRUE(ctx, elem.contains({100.0f, 100.0f}));
        ERGO_TEST_ASSERT_TRUE(ctx, elem.contains({300.0f, 250.0f}));
        ERGO_TEST_ASSERT_FALSE(ctx, elem.contains({50.0f, 50.0f}));
        ERGO_TEST_ASSERT_FALSE(ctx, elem.contains({350.0f, 350.0f}));
    });

    ui_element.add("UIElement_ComputedPosition_NoParent", [](TestContext& ctx) {
        UIElement elem;
        elem.position = {10.0f, 20.0f};
        elem.margin = {5.0f, 5.0f};

        Vec2f pos = elem.computed_position();
        ERGO_TEST_ASSERT_NEAR(ctx, pos.x, 15.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, pos.y, 25.0f, 0.01f);
    });

    ui_element.add("UIElement_ComputedPosition_WithParent_TopLeft", [](TestContext& ctx) {
        UIElement parent;
        parent.position = {100.0f, 100.0f};
        parent.size = {400.0f, 300.0f};

        UIElement child;
        child.position = {10.0f, 10.0f};
        child.anchor = Anchor::TopLeft;
        child.parent = &parent;

        Vec2f pos = child.computed_position();
        ERGO_TEST_ASSERT_NEAR(ctx, pos.x, 110.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, pos.y, 110.0f, 0.01f);
    });

    ui_element.add("UIElement_ComputedPosition_WithParent_Center", [](TestContext& ctx) {
        UIElement parent;
        parent.position = {100.0f, 100.0f};
        parent.size = {400.0f, 300.0f};

        UIElement child;
        child.position = {0.0f, 0.0f};
        child.anchor = Anchor::Center;
        child.parent = &parent;

        Vec2f pos = child.computed_position();
        ERGO_TEST_ASSERT_NEAR(ctx, pos.x, 300.0f, 0.01f);  // 100 + 400*0.5
        ERGO_TEST_ASSERT_NEAR(ctx, pos.y, 250.0f, 0.01f);  // 100 + 300*0.5
    });

    ui_element.add("UILabel_Properties", [](TestContext& ctx) {
        UILabel label;
        label.text = "Hello";
        label.color = {255, 0, 0, 255};
        label.font_scale = 2.0f;

        ERGO_TEST_ASSERT_TRUE(ctx, label.text == "Hello");
        ERGO_TEST_ASSERT_EQ(ctx, label.color.r, (uint8_t)255);
        ERGO_TEST_ASSERT_NEAR(ctx, label.font_scale, 2.0f, 0.001f);
    });

    ui_element.add("UIButton_Callback", [](TestContext& ctx) {
        UIButton btn;
        btn.text = "Click";
        bool clicked = false;
        btn.on_click = [&clicked]() { clicked = true; };

        ERGO_TEST_ASSERT_FALSE(ctx, clicked);
        btn.on_click();
        ERGO_TEST_ASSERT_TRUE(ctx, clicked);
    });

    ui_element.add("UISlider_Range", [](TestContext& ctx) {
        UISlider slider;
        slider.min_value = 0.0f;
        slider.max_value = 100.0f;
        slider.value = 50.0f;

        ERGO_TEST_ASSERT_NEAR(ctx, slider.value, 50.0f, 0.001f);
        ERGO_TEST_ASSERT_TRUE(ctx, slider.value >= slider.min_value);
        ERGO_TEST_ASSERT_TRUE(ctx, slider.value <= slider.max_value);
    });

    ui_element.add("UIProgressBar_Bounds", [](TestContext& ctx) {
        UIProgressBar bar;
        bar.progress = 0.75f;
        ERGO_TEST_ASSERT_NEAR(ctx, bar.progress, 0.75f, 0.001f);
    });

    return 0;
}();

// ---------------------------------------------------------------------------
// Shader/Graph suite
// ---------------------------------------------------------------------------
static TestSuite shader_graph("Shader/Graph");

static auto shader_init = []() {
    shader_graph.add("ShaderGraph_AddNode", [](TestContext& ctx) {
        ShaderGraph graph;
        auto id = graph.add_node(ShaderNodeLibrary::create_float_property("u_val", 1.0f));
        ERGO_TEST_ASSERT_TRUE(ctx, id > 0);
        ERGO_TEST_ASSERT_EQ(ctx, graph.node_count(), (size_t)1);
    });

    shader_graph.add("ShaderGraph_Connect", [](TestContext& ctx) {
        ShaderGraph graph;
        auto a = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 1.0f));
        auto b = graph.add_node(ShaderNodeLibrary::create_output());
        graph.connect(a, 0, b, 0);
        ERGO_TEST_ASSERT_EQ(ctx, graph.connection_count(), (size_t)1);
    });

    shader_graph.add("ShaderGraph_Validate", [](TestContext& ctx) {
        ShaderGraph graph;
        auto color = graph.add_node(ShaderNodeLibrary::create_color_property("u_color", 1, 0, 0, 1));
        auto output = graph.add_node(ShaderNodeLibrary::create_output());
        graph.connect(color, 0, output, 0);
        ERGO_TEST_ASSERT_TRUE(ctx, graph.validate());
    });

    shader_graph.add("ShaderGraph_TopologicalSort", [](TestContext& ctx) {
        ShaderGraph graph;
        auto a = graph.add_node(ShaderNodeLibrary::create_float_property("u_a", 1.0f));
        auto b = graph.add_node(ShaderNodeLibrary::create_float_property("u_b", 2.0f));
        auto add = graph.add_node(ShaderNodeLibrary::create_math(MathOp::Add));
        auto out = graph.add_node(ShaderNodeLibrary::create_output());

        graph.connect(a, 0, add, 0);
        graph.connect(b, 0, add, 1);
        graph.connect(add, 0, out, 0);

        auto order = graph.topological_sort();
        ERGO_TEST_ASSERT_EQ(ctx, order.size(), (size_t)4);

        // Output should be last
        ERGO_TEST_ASSERT_EQ(ctx, order.back(), out);
    });

    shader_graph.add("ShaderCompiler_GenerateGLSL", [](TestContext& ctx) {
        ShaderGraph graph;
        auto color = graph.add_node(ShaderNodeLibrary::create_color_property("u_baseColor", 1, 1, 1, 1));
        auto output = graph.add_node(ShaderNodeLibrary::create_output());
        graph.connect(color, 0, output, 0);

        ShaderCompiler compiler(ShaderLanguage::GLSL_450);
        std::string vert = compiler.generate_vertex(graph);
        std::string frag = compiler.generate_fragment(graph);

        ERGO_TEST_ASSERT_TRUE(ctx, vert.size() > 0);
        ERGO_TEST_ASSERT_TRUE(ctx, frag.size() > 0);
    });

    shader_graph.add("ShaderOptimizer_RemoveUnused", [](TestContext& ctx) {
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
        ERGO_TEST_ASSERT_TRUE(ctx, report.size() > 0);
    });

    return 0;
}();

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_ui_shader_tests(TestRunner& runner) {
    runner.add_suite(ui_element);
    runner.add_suite(shader_graph);
}
