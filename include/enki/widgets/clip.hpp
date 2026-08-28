#pragma once
/// @file clip.hpp
/// @brief High-performance Skia clipping widgets: ClipRect, ClipRRect, ClipOval, ClipPath.
///
/// 100% C++20 Declarative Syntax with designated initializers.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/path.hpp"
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

/// Custom clipper callback mapping a layout size to a geometric path
using CustomClipper = std::function<Path(Size)>;

// ════════════════════════════════════════════════════════════════
// ClipRect
// ════════════════════════════════════════════════════════════════

class ClipRectWidget : public SingleChildRenderObjectWidget {
public:
    Clip clip_behavior = Clip::AntiAlias;

    ClipRectWidget() = default;
    explicit ClipRectWidget(WidgetPtr child, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), clip_behavior(clip) {}
    ClipRectWidget(Key key, WidgetPtr child, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), clip_behavior(clip) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ClipRect"; }
};

struct ClipRect {
    Clip      clip_behavior = Clip::AntiAlias;
    WidgetPtr child = nullptr;
    Key       key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ClipRectWidget>(key, child, clip_behavior);
    }
};

inline std::shared_ptr<ClipRectWidget> clipRect(const ClipRect& props) {
    return std::make_shared<ClipRectWidget>(props.key, props.child, props.clip_behavior);
}

// ════════════════════════════════════════════════════════════════
// ClipRRect
// ════════════════════════════════════════════════════════════════

class ClipRRectWidget : public SingleChildRenderObjectWidget {
public:
    BorderRadius border_radius = BorderRadius::zero();
    Clip         clip_behavior = Clip::AntiAlias;

    ClipRRectWidget() = default;
    explicit ClipRRectWidget(WidgetPtr child, BorderRadius radius = BorderRadius::zero(), Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), border_radius(radius), clip_behavior(clip) {}
    ClipRRectWidget(Key key, WidgetPtr child, BorderRadius radius = BorderRadius::zero(), Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), border_radius(radius), clip_behavior(clip) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ClipRRect"; }
};

struct ClipRRect {
    BorderRadius border_radius = BorderRadius::zero();
    Clip         clip_behavior = Clip::AntiAlias;
    WidgetPtr    child = nullptr;
    Key          key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ClipRRectWidget>(key, child, border_radius, clip_behavior);
    }
};

inline std::shared_ptr<ClipRRectWidget> clipRRect(const ClipRRect& props) {
    return std::make_shared<ClipRRectWidget>(props.key, props.child, props.border_radius, props.clip_behavior);
}

// ════════════════════════════════════════════════════════════════
// ClipOval
// ════════════════════════════════════════════════════════════════

class ClipOvalWidget : public SingleChildRenderObjectWidget {
public:
    Clip clip_behavior = Clip::AntiAlias;

    ClipOvalWidget() = default;
    explicit ClipOvalWidget(WidgetPtr child, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), clip_behavior(clip) {}
    ClipOvalWidget(Key key, WidgetPtr child, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), clip_behavior(clip) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ClipOval"; }
};

struct ClipOval {
    Clip      clip_behavior = Clip::AntiAlias;
    WidgetPtr child = nullptr;
    Key       key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ClipOvalWidget>(key, child, clip_behavior);
    }
};

inline std::shared_ptr<ClipOvalWidget> clipOval(const ClipOval& props) {
    return std::make_shared<ClipOvalWidget>(props.key, props.child, props.clip_behavior);
}

// ════════════════════════════════════════════════════════════════
// ClipPath
// ════════════════════════════════════════════════════════════════

class ClipPathWidget : public SingleChildRenderObjectWidget {
public:
    CustomClipper         clipper = nullptr;
    std::shared_ptr<Path> path = nullptr;
    Clip                  clip_behavior = Clip::AntiAlias;

    ClipPathWidget() = default;
    explicit ClipPathWidget(WidgetPtr child, CustomClipper clipper = nullptr, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), clipper(std::move(clipper)), clip_behavior(clip) {}
    ClipPathWidget(Key key, WidgetPtr child, CustomClipper clipper = nullptr, std::shared_ptr<Path> path = nullptr, Clip clip = Clip::AntiAlias)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), clipper(std::move(clipper)), path(std::move(path)), clip_behavior(clip) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ClipPath"; }
};

struct ClipPath {
    CustomClipper         clipper = nullptr;
    std::shared_ptr<Path> path = nullptr;
    Clip                  clip_behavior = Clip::AntiAlias;
    WidgetPtr             child = nullptr;
    Key                   key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ClipPathWidget>(key, child, clipper, path, clip_behavior);
    }
};

inline std::shared_ptr<ClipPathWidget> clipPath(const ClipPath& props) {
    return std::static_pointer_cast<ClipPathWidget>(static_cast<WidgetPtr>(props));
}

} // namespace enki
