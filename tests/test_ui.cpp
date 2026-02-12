#include "test_framework.hpp"
#include "engine/ui/ui_element.hpp"
#include "engine/ui/ui_widgets.hpp"

TEST_CASE(UIElement_Contains) {
    UIElement elem;
    elem.position = {100.0f, 100.0f};
    elem.size = {200.0f, 150.0f};

    ASSERT_TRUE(elem.contains({150.0f, 150.0f}));
    ASSERT_TRUE(elem.contains({100.0f, 100.0f}));
    ASSERT_TRUE(elem.contains({300.0f, 250.0f}));
    ASSERT_FALSE(elem.contains({50.0f, 50.0f}));
    ASSERT_FALSE(elem.contains({350.0f, 350.0f}));
}

TEST_CASE(UIElement_ComputedPosition_NoParent) {
    UIElement elem;
    elem.position = {10.0f, 20.0f};
    elem.margin = {5.0f, 5.0f};

    Vec2f pos = elem.computed_position();
    ASSERT_NEAR(pos.x, 15.0f, 0.01f);
    ASSERT_NEAR(pos.y, 25.0f, 0.01f);
}

TEST_CASE(UIElement_ComputedPosition_WithParent_TopLeft) {
    UIElement parent;
    parent.position = {100.0f, 100.0f};
    parent.size = {400.0f, 300.0f};

    UIElement child;
    child.position = {10.0f, 10.0f};
    child.anchor = Anchor::TopLeft;
    child.parent = &parent;

    Vec2f pos = child.computed_position();
    ASSERT_NEAR(pos.x, 110.0f, 0.01f);
    ASSERT_NEAR(pos.y, 110.0f, 0.01f);
}

TEST_CASE(UIElement_ComputedPosition_WithParent_Center) {
    UIElement parent;
    parent.position = {100.0f, 100.0f};
    parent.size = {400.0f, 300.0f};

    UIElement child;
    child.position = {0.0f, 0.0f};
    child.anchor = Anchor::Center;
    child.parent = &parent;

    Vec2f pos = child.computed_position();
    ASSERT_NEAR(pos.x, 300.0f, 0.01f);  // 100 + 400*0.5
    ASSERT_NEAR(pos.y, 250.0f, 0.01f);  // 100 + 300*0.5
}

TEST_CASE(UILabel_Properties) {
    UILabel label;
    label.text = "Hello";
    label.color = {255, 0, 0, 255};
    label.font_scale = 2.0f;

    ASSERT_TRUE(label.text == "Hello");
    ASSERT_EQ(label.color.r, (uint8_t)255);
    ASSERT_NEAR(label.font_scale, 2.0f, 0.001f);
}

TEST_CASE(UIButton_Callback) {
    UIButton btn;
    btn.text = "Click";
    bool clicked = false;
    btn.on_click = [&clicked]() { clicked = true; };

    ASSERT_FALSE(clicked);
    btn.on_click();
    ASSERT_TRUE(clicked);
}

TEST_CASE(UISlider_Range) {
    UISlider slider;
    slider.min_value = 0.0f;
    slider.max_value = 100.0f;
    slider.value = 50.0f;

    ASSERT_NEAR(slider.value, 50.0f, 0.001f);
    ASSERT_TRUE(slider.value >= slider.min_value);
    ASSERT_TRUE(slider.value <= slider.max_value);
}

TEST_CASE(UIProgressBar_Bounds) {
    UIProgressBar bar;
    bar.progress = 0.75f;
    ASSERT_NEAR(bar.progress, 0.75f, 0.001f);
}
