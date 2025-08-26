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
/// Focus Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct FocusProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    std::shared_ptr<FocusNode> focus_node = nullptr;
    bool autofocus = false;
    bool show_focus_ring = true;
    Color focus_ring_color = 0xFF38BDF8;

    std::function<void(bool has_focus)> on_focus_change;
    std::function<void(int key, int mod)> on_key;
};

class FocusWidget : public StatefulWidget {
public:
    WidgetPtr child;
    std::shared_ptr<FocusNode> focus_node;
    bool autofocus = false;
    bool show_focus_ring = true;
    Color focus_ring_color = 0xFF38BDF8;

    std::function<void(bool has_focus)> on_focus_change;
    std::function<void(int key, int mod)> on_key;

    FocusWidget() = default;
    FocusWidget(WidgetPtr child_, std::shared_ptr<FocusNode> node = nullptr, bool auto_foc = false)
        : child(std::move(child_)), focus_node(std::move(node)), autofocus(auto_foc) {}
    FocusWidget(Key key, WidgetPtr child_, std::shared_ptr<FocusNode> node = nullptr, bool auto_foc = false)
        : StatefulWidget(std::move(key)), child(std::move(child_)), focus_node(std::move(node)), autofocus(auto_foc) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Focus"; }
};

/// ════════════════════════════════════════════════════════════════
/// FocusScope Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct FocusScopeProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    bool autofocus = false;
};

class FocusScopeWidget : public StatefulWidget {
public:
    WidgetPtr child;
    bool autofocus = false;

    FocusScopeWidget() = default;
    explicit FocusScopeWidget(WidgetPtr child_, bool auto_foc = false)
        : child(std::move(child_)), autofocus(auto_foc) {}
    FocusScopeWidget(Key key, WidgetPtr child_, bool auto_foc = false)
        : StatefulWidget(std::move(key)), child(std::move(child_)), autofocus(auto_foc) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FocusScope"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Structs (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Focus {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    std::shared_ptr<FocusNode> focus_node = nullptr;
    bool autofocus = false;
    bool show_focus_ring = true;
    Color focus_ring_color = 0xFF38BDF8;

    std::function<void(bool has_focus)> on_focus_change = nullptr;
    std::function<void(int key, int mod)> on_key = nullptr;

    operator WidgetPtr() const {
        auto w = std::make_shared<FocusWidget>(key, child, focus_node, autofocus);
        w->show_focus_ring = show_focus_ring;
        w->focus_ring_color = focus_ring_color;
        w->on_focus_change = on_focus_change;
        w->on_key = on_key;
        return w;
    }
};

struct FocusScope {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    bool autofocus = false;

    operator WidgetPtr() const {
        return std::make_shared<FocusScopeWidget>(key, child, autofocus);
    }
};

} // namespace enki
