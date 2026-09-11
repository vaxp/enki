#pragma once
/// @file safe_area.hpp
/// @brief SafeArea widget — pads its child to avoid system UI intrusions.
///
/// On Android, safe area insets are read from Platform::getSafeAreaInsets() which
/// queries status_bar_height and navigation_bar_height via JNI.
/// On desktop the insets are always zero, making SafeArea a transparent no-op.
///
/// Usage:
/// @code
///   safeArea(myContent);                     // respect all sides
///   safeArea(myContent, true, false);        // top only (ignore bottom)
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/platform/platform.hpp"
#include "enki/widgets/container.hpp"
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// SafeArea
// ════════════════════════════════════════════════════════════════

class SafeArea : public StatefulWidget {
public:
    WidgetPtr child   = nullptr;
    bool top          = true;   ///< Respect top inset (status bar / notch).
    bool bottom       = true;   ///< Respect bottom inset (navigation bar / gesture strip).
    bool left         = true;   ///< Respect left inset (punch-hole cutout).
    bool right        = true;   ///< Respect right inset (punch-hole cutout).

    explicit SafeArea(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "SafeArea"; }
};

class SafeAreaState : public State {
public:
    WidgetPtr build(BuildContext& /*ctx*/) override {
        const auto* w = static_cast<const SafeArea*>(widget());

        EdgeInsets insets;
        if (Platform* plat = Platform::instance()) {
            insets = plat->getSafeAreaInsets();
        }

        StyleInsets padding;
        padding.top    = w->top    ? StyleValue::point(insets.top)    : StyleValue{};
        padding.bottom = w->bottom ? StyleValue::point(insets.bottom) : StyleValue{};
        padding.left   = w->left   ? StyleValue::point(insets.left)   : StyleValue{};
        padding.right  = w->right  ? StyleValue::point(insets.right)  : StyleValue{};

        ContainerProps cp;
        cp.width   = StyleValue::percent(100.0f);
        cp.height  = StyleValue::percent(100.0f);
        cp.padding = padding;
        cp.child   = w->child;
        return container(cp);
    }
};

inline std::unique_ptr<State> SafeArea::createState() {
    return std::make_unique<SafeAreaState>();
}

/// @brief Convenience factory: wrap a child in a SafeArea.
/// @param child     The child widget to protect.
/// @param top       Whether to apply top inset (default: true).
/// @param bottom    Whether to apply bottom inset (default: true).
inline WidgetPtr safeArea(WidgetPtr child, bool top = true, bool bottom = true,
                           bool left = true, bool right = true) {
    auto w = std::make_shared<SafeArea>();
    w->child  = std::move(child);
    w->top    = top;
    w->bottom = bottom;
    w->left   = left;
    w->right  = right;
    return w;
}

} // namespace enki
