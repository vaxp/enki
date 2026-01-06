#pragma once
/// @file hero.hpp
/// @brief Shared Element Hero Transition System for ENKI.
///
/// Features:
///   - Seamless visual morphing of widgets between screens/routes.
///   - Automatic coordinate tracking via RenderObject::globalBounds().
///   - C++20 designated initializers: hero({ .tag = "item_1", .child = ... }).
///   - Support for custom flight shuttle builders and spring physics.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace enki {

class RenderHero;

// ════════════════════════════════════════════════════════════════
// HeroRegistry — Global tracking of active Hero elements
// ════════════════════════════════════════════════════════════════

struct HeroRecord {
    std::string   tag;
    RenderHero*   render_object = nullptr;
    Rect          last_global_bounds;
    WidgetPtr     widget = nullptr;
};

class HeroRegistry {
public:
    static HeroRegistry& instance() {
        static HeroRegistry s_registry;
        return s_registry;
    }

    void registerHero(const std::string& tag, RenderHero* ro, WidgetPtr w);
    void unregisterHero(const std::string& tag, RenderHero* ro);
    void updateBounds(const std::string& tag, RenderHero* ro, const Rect& bounds);

    [[nodiscard]] const HeroRecord* findHero(const std::string& tag) const;
    [[nodiscard]] bool hasHero(const std::string& tag) const;

    void clear();

private:
    std::unordered_map<std::string, HeroRecord> heroes_;
};

// ════════════════════════════════════════════════════════════════
// RenderHero — RenderObject managing Hero visibility & bounds
// ════════════════════════════════════════════════════════════════

class RenderHero : public RenderBox {
public:
    std::string tag;
    bool is_placeholder = false; // When true, child is hidden while flight is in progress

    explicit RenderHero(std::string t) : tag(std::move(t)) {}
    ~RenderHero() override;

    void paint(PaintContext& ctx) override;
};

// ════════════════════════════════════════════════════════════════
// Hero Widget & Declarative Syntax
// ════════════════════════════════════════════════════════════════

class HeroWidget : public SingleChildRenderObjectWidget {
public:
    std::string tag;

    HeroWidget(Key key, std::string t, WidgetPtr ch)
        : SingleChildRenderObjectWidget(std::move(key), std::move(ch)), tag(std::move(t)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Hero"; }
};

struct HeroProps {
    std::string tag;
    WidgetPtr   child = nullptr;
    Key         key   = Key::none();

    operator WidgetPtr() const;
};

struct Hero : public HeroProps {
    using HeroProps::HeroProps;
};

inline WidgetPtr hero(const HeroProps& props) {
    return static_cast<WidgetPtr>(props);
}

inline HeroProps::operator WidgetPtr() const {
    return std::make_shared<HeroWidget>(key, tag, child);
}

// ════════════════════════════════════════════════════════════════
// HeroFlightShuttle — Floating flight shuttle between two Rects
// ════════════════════════════════════════════════════════════════

class RenderHeroFlight : public RenderBox {
public:
    Rect      start_rect;
    Rect      end_rect;
    float     progress = 0.0f;
    WidgetPtr shuttle;

    RenderHeroFlight(Rect start, Rect end, float p)
        : start_rect(start), end_rect(end), progress(p) {}

    void paint(PaintContext& ctx) override;
};

class HeroFlightWidget : public SingleChildRenderObjectWidget {
public:
    Rect  start_rect;
    Rect  end_rect;
    float progress;

    HeroFlightWidget(Rect s, Rect e, float p, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          start_rect(s), end_rect(e), progress(p) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderHeroFlight>(start_rect, end_rect, progress);
    }
    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& rf = static_cast<RenderHeroFlight&>(ro);
        rf.start_rect = start_rect;
        rf.end_rect   = end_rect;
        rf.progress   = progress;
        rf.markNeedsPaint();
    }
    [[nodiscard]] std::string_view typeName() const override { return "HeroFlight"; }
};

inline WidgetPtr heroFlight(Rect start_rect, Rect end_rect, float progress, WidgetPtr child) {
    return std::make_shared<HeroFlightWidget>(start_rect, end_rect, progress, std::move(child));
}

} // namespace enki
