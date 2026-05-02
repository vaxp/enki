#pragma once
/// @file aspect_ratio.hpp
/// @brief AspectRatio widget — constrains a child to a specific width-to-height ratio.
///
/// The child is given a width equal to the parent's available width, and the height
/// is calculated as `width / ratio`. If the parent constrains height below that
/// value, the widget shrinks proportionally.
///
/// Usage:
/// @code
///   aspectRatio(1.0f, mySquareContent);        // 1:1 square
///   aspectRatio(16.0f/9.0f, videoFrame);       // 16:9 widescreen
///   aspectRatio(1.25f, calcButton);            // calculator button shape
/// @endcode
///
/// Implementation note: AspectRatio is a thin StatelessWidget wrapper that forwards
/// to container() with FlexboxStyle::aspect_ratio set, which is natively understood
/// by the Anu Layout Engine — zero custom measurement code.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/container.hpp"
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// AspectRatio
// ════════════════════════════════════════════════════════════════

class AspectRatioWidget : public StatelessWidget {
public:
    float     ratio = 1.0f;   ///< Width / Height ratio (e.g. 1.77 for 16:9).
    WidgetPtr child = nullptr;

    explicit AspectRatioWidget(Key key = Key::none()) : StatelessWidget(std::move(key)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& /*ctx*/) override {
        ContainerProps p;
        p.aspect_ratio = ratio;
        p.width        = StyleValue::percent(100.0f);
        p.child        = child;
        return container(p);
    }

    [[nodiscard]] std::string_view typeName() const override { return "AspectRatio"; }
};

struct AspectRatioProps {
    Key       key   = Key::none();
    float     ratio = 1.0f;
    WidgetPtr child = nullptr;
};

/// @brief Convenience factory: wrap child in an AspectRatio constraint.
inline WidgetPtr aspectRatio(float ratio, WidgetPtr child, Key key = Key::none()) {
    auto w = std::make_shared<AspectRatioWidget>(std::move(key));
    w->ratio = ratio;
    w->child = std::move(child);
    return w;
}

inline WidgetPtr aspectRatio(const AspectRatioProps& props) {
    return aspectRatio(props.ratio, props.child, props.key);
}

} // namespace enki
