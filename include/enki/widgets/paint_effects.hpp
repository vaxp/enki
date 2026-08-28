#pragma once
/// @file paint_effects.hpp
/// @brief Visual & Paint effects widgets: BackdropFilter, DecoratedBox, ShaderMask, ColorFiltered.
///
/// 100% C++20 Declarative Syntax with designated initializers.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/widgets/container.hpp"
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

enum class DecorationPosition {
    Background,
    Foreground,
};

using ShaderCallback = std::function<std::shared_ptr<Shader>(Rect bounds)>;

// ════════════════════════════════════════════════════════════════
// BackdropFilter
// ════════════════════════════════════════════════════════════════

class BackdropFilterWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<ImageFilter> filter;
    BlendMode                    blend_mode = BlendMode::SrcOver;

    BackdropFilterWidget() = default;
    explicit BackdropFilterWidget(WidgetPtr child, std::shared_ptr<ImageFilter> f = nullptr, BlendMode mode = BlendMode::SrcOver)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), filter(std::move(f)), blend_mode(mode) {}
    BackdropFilterWidget(Key key, WidgetPtr child, std::shared_ptr<ImageFilter> f = nullptr, BlendMode mode = BlendMode::SrcOver)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), filter(std::move(f)), blend_mode(mode) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "BackdropFilter"; }
};

struct BackdropFilter {
    std::shared_ptr<ImageFilter> filter = nullptr;
    BlendMode                    blend_mode = BlendMode::SrcOver;
    WidgetPtr                    child = nullptr;
    Key                          key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<BackdropFilterWidget>(key, child, filter, blend_mode);
    }
};

inline std::shared_ptr<BackdropFilterWidget> backdropFilter(const BackdropFilter& props) {
    return std::make_shared<BackdropFilterWidget>(props.key, props.child, props.filter, props.blend_mode);
}

// ════════════════════════════════════════════════════════════════
// DecoratedBox
// ════════════════════════════════════════════════════════════════

class DecoratedBoxWidget : public SingleChildRenderObjectWidget {
public:
    BoxDecoration      decoration;
    DecorationPosition position = DecorationPosition::Background;

    DecoratedBoxWidget() = default;
    explicit DecoratedBoxWidget(WidgetPtr child, BoxDecoration dec = {}, DecorationPosition pos = DecorationPosition::Background)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), decoration(std::move(dec)), position(pos) {}
    DecoratedBoxWidget(Key key, WidgetPtr child, BoxDecoration dec = {}, DecorationPosition pos = DecorationPosition::Background)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), decoration(std::move(dec)), position(pos) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "DecoratedBox"; }
};

struct DecoratedBox {
    BoxDecoration      decoration = {};
    DecorationPosition position = DecorationPosition::Background;
    WidgetPtr          child = nullptr;
    Key                key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<DecoratedBoxWidget>(key, child, decoration, position);
    }
};

inline std::shared_ptr<DecoratedBoxWidget> decoratedBox(const DecoratedBox& props) {
    return std::make_shared<DecoratedBoxWidget>(props.key, props.child, props.decoration, props.position);
}

// ════════════════════════════════════════════════════════════════
// ShaderMask
// ════════════════════════════════════════════════════════════════

class ShaderMaskWidget : public SingleChildRenderObjectWidget {
public:
    ShaderCallback shader_callback;
    BlendMode      blend_mode = BlendMode::Modulate;

    ShaderMaskWidget() = default;
    explicit ShaderMaskWidget(WidgetPtr child, ShaderCallback cb = nullptr, BlendMode mode = BlendMode::Modulate)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), shader_callback(std::move(cb)), blend_mode(mode) {}
    ShaderMaskWidget(Key key, WidgetPtr child, ShaderCallback cb = nullptr, BlendMode mode = BlendMode::Modulate)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), shader_callback(std::move(cb)), blend_mode(mode) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ShaderMask"; }
};

struct ShaderMask {
    ShaderCallback shader_callback = nullptr;
    BlendMode      blend_mode = BlendMode::Modulate;
    WidgetPtr      child = nullptr;
    Key            key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ShaderMaskWidget>(key, child, shader_callback, blend_mode);
    }
};

inline std::shared_ptr<ShaderMaskWidget> shaderMask(const ShaderMask& props) {
    return std::make_shared<ShaderMaskWidget>(props.key, props.child, props.shader_callback, props.blend_mode);
}

// ════════════════════════════════════════════════════════════════
// ColorFiltered
// ════════════════════════════════════════════════════════════════

class ColorFilteredWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<ColorFilter> color_filter;

    ColorFilteredWidget() = default;
    explicit ColorFilteredWidget(WidgetPtr child, std::shared_ptr<ColorFilter> filter = nullptr)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), color_filter(std::move(filter)) {}
    ColorFilteredWidget(Key key, WidgetPtr child, std::shared_ptr<ColorFilter> filter = nullptr)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), color_filter(std::move(filter)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "ColorFiltered"; }
};

struct ColorFiltered {
    std::shared_ptr<ColorFilter> color_filter = nullptr;
    WidgetPtr                    child = nullptr;
    Key                          key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<ColorFilteredWidget>(key, child, color_filter);
    }
};

inline std::shared_ptr<ColorFilteredWidget> colorFiltered(const ColorFiltered& props) {
    return std::make_shared<ColorFilteredWidget>(props.key, props.child, props.color_filter);
}

} // namespace enki
