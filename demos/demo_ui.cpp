#include "demo_framework.hpp"
#include "engine/ui/ui_element.hpp"
#include "engine/ui/ui_widgets.hpp"
#include <cstdio>

DEMO(UI_Element_Hierarchy) {
    UIElement root;
    root.id = 1;
    root.position = {10.0f, 10.0f};
    root.size = {400.0f, 300.0f};
    root.anchor = Anchor::TopLeft;

    UIElement child;
    child.id = 2;
    child.position = {0.0f, 0.0f};
    child.size = {100.0f, 50.0f};
    child.anchor = Anchor::Center;
    child.parent = &root;

    root.children.push_back(&child);

    Vec2f root_pos = root.computed_position();
    Vec2f child_pos = child.computed_position();
    std::printf("  Root computed pos: (%.1f, %.1f)\n", root_pos.x, root_pos.y);
    std::printf("  Child (Center anchor) computed pos: (%.1f, %.1f)\n",
                child_pos.x, child_pos.y);

    // Hit test
    std::printf("  Root contains (50,50): %s\n",
                root.contains({50.0f, 50.0f}) ? "yes" : "no");
    std::printf("  Root contains (500,500): %s\n",
                root.contains({500.0f, 500.0f}) ? "yes" : "no");
}

DEMO(UI_Widgets) {
    UILabel label;
    label.id = 10;
    label.text = "Score: 1234";
    label.color = {255, 255, 0, 255};
    label.font_scale = 2.0f;
    label.position = {10.0f, 10.0f};
    label.size = {200.0f, 30.0f};
    std::printf("  Label: '%s' color=(%d,%d,%d) scale=%.1f\n",
                label.text.c_str(), label.color.r, label.color.g, label.color.b,
                label.font_scale);

    UIButton btn;
    btn.id = 11;
    btn.text = "Start Game";
    btn.position = {100.0f, 100.0f};
    btn.size = {200.0f, 50.0f};
    bool clicked = false;
    btn.on_click = [&clicked]() { clicked = true; };
    std::printf("  Button: '%s' at (%.1f,%.1f) size=(%.1f,%.1f)\n",
                btn.text.c_str(), btn.position.x, btn.position.y,
                btn.size.w, btn.size.h);

    // Simulate click
    btn.on_click();
    std::printf("  Button clicked: %s\n", clicked ? "yes" : "no");

    UISlider slider;
    slider.id = 12;
    slider.min_value = 0.0f;
    slider.max_value = 100.0f;
    slider.value = 75.0f;
    std::printf("  Slider: value=%.1f range=[%.1f, %.1f]\n",
                slider.value, slider.min_value, slider.max_value);

    UIProgressBar progress;
    progress.id = 13;
    progress.progress = 0.6f;
    std::printf("  ProgressBar: %.0f%%\n", progress.progress * 100.0f);

    UITextInput input;
    input.id = 14;
    input.placeholder = "Enter name...";
    input.text = "Player1";
    std::printf("  TextInput: text='%s' placeholder='%s'\n",
                input.text.c_str(), input.placeholder.c_str());
}
