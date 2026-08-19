#pragma once
/// @file focus.hpp
/// @brief Focus and FocusScope widgets for keyboard navigation & focus rings in ENKI Framework (Category 9. Gestures / Interaction).
/// Supports FocusNode management, global FocusManager, Tab/Shift+Tab focus traversal, and visual focus halo indicators.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>

namespace enki {

class FocusNode;

/// ════════════════════════════════════════════════════════════════
/// Global FocusManager
/// ════════════════════════════════════════════════════════════════

class FocusManager {
public:
    static FocusManager& instance() {
        static FocusManager mgr;
        return mgr;
    }

    FocusNode* current_focus = nullptr;

    void setFocus(FocusNode* node);
    void clearFocus();
};

/// ════════════════════════════════════════════════════════════════
/// FocusNode
/// ════════════════════════════════════════════════════════════════

class FocusNode {
public:
    bool has_focus = false;
    bool can_request_focus = true;
    std::string debug_label = "";

    std::function<void(bool has_focus)> on_focus_changed;

    FocusNode() = default;
    ~FocusNode() {
        if (FocusManager::instance().current_focus == this) {
            FocusManager::instance().current_focus = nullptr;
        }
    }

    void requestFocus() {
        FocusManager::instance().setFocus(this);
    }

    void unfocus() {
        if (FocusManager::instance().current_focus == this) {
            FocusManager::instance().clearFocus();
        } else if (has_focus) {
            has_focus = false;
            if (on_focus_changed) on_focus_changed(false);
        }
    }
};

/// ════════════════════════════════════════════════════════════════
/// Focus Widget
/// ════════════════════════════════════════════════════════════════

class Focus : public StatefulWidget {
public:
    WidgetPtr child;
    std::shared_ptr<FocusNode> focus_node;
    bool autofocus = false;
    bool show_focus_ring = true;
    Color focus_ring_color = 0xFF38BDF8;

    std::function<void(bool has_focus)> on_focus_change;
    std::function<void(int key, int mod)> on_key;

    Focus() = default;
    Focus(WidgetPtr child_, std::shared_ptr<FocusNode> node = nullptr, bool auto_foc = false)
        : child(std::move(child_)), focus_node(std::move(node)), autofocus(auto_foc) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Focus"; }
};

inline std::shared_ptr<Focus> focus(
    WidgetPtr child,
    std::shared_ptr<FocusNode> node = nullptr,
    bool autofocus = false) {
    return std::make_shared<Focus>(std::move(child), std::move(node), autofocus);
}

/// ════════════════════════════════════════════════════════════════
/// FocusScope Widget
/// ════════════════════════════════════════════════════════════════

class FocusScope : public StatefulWidget {
public:
    WidgetPtr child;
    bool autofocus = false;

    FocusScope() = default;
    explicit FocusScope(WidgetPtr child_, bool auto_foc = false)
        : child(std::move(child_)), autofocus(auto_foc) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FocusScope"; }
};

inline std::shared_ptr<FocusScope> focusScope(WidgetPtr child, bool autofocus = false) {
    return std::make_shared<FocusScope>(std::move(child), autofocus);
}

} // namespace enki
