#pragma once
#include "../math/vec2.hpp"
#include "../math/size2.hpp"
#include "../math/color.hpp"
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

enum class Anchor : uint8_t {
    TopLeft, Top, TopRight,
    Left, Center, Right,
    BottomLeft, Bottom, BottomRight
};

struct UIElement {
    uint64_t id = 0;
    Vec2f position;
    Size2f size;
    Anchor anchor = Anchor::TopLeft;
    Vec2f margin;
    bool visible = true;
    bool interactive = true;

    UIElement* parent = nullptr;
    std::vector<UIElement*> children;

    std::function<void()> on_click;
    std::function<void()> on_hover_enter;
    std::function<void()> on_hover_exit;

    Vec2f computed_position() const {
        Vec2f base = position;
        if (parent) {
            Vec2f pp = parent->computed_position();
            Size2f ps = parent->size;
            switch (anchor) {
                case Anchor::TopLeft:     base += pp; break;
                case Anchor::Top:         base += {pp.x + ps.w * 0.5f, pp.y}; break;
                case Anchor::TopRight:    base += {pp.x + ps.w, pp.y}; break;
                case Anchor::Left:        base += {pp.x, pp.y + ps.h * 0.5f}; break;
                case Anchor::Center:      base += {pp.x + ps.w * 0.5f, pp.y + ps.h * 0.5f}; break;
                case Anchor::Right:       base += {pp.x + ps.w, pp.y + ps.h * 0.5f}; break;
                case Anchor::BottomLeft:  base += {pp.x, pp.y + ps.h}; break;
                case Anchor::Bottom:      base += {pp.x + ps.w * 0.5f, pp.y + ps.h}; break;
                case Anchor::BottomRight: base += {pp.x + ps.w, pp.y + ps.h}; break;
            }
        }
        return base + margin;
    }

    bool contains(Vec2f point) const {
        Vec2f cp = computed_position();
        return point.x >= cp.x && point.x <= cp.x + size.w &&
               point.y >= cp.y && point.y <= cp.y + size.h;
    }

    virtual ~UIElement() = default;
    virtual void update(float /*dt*/) {}
    virtual void draw(struct RenderContext& /*ctx*/) {}
};
