/// @file focus.cpp
/// @brief Implementation of Focus and FocusScope widgets for ENKI Framework.

#include "enki/widgets/focus.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// FocusManager Implementation
// ════════════════════════════════════════════════════════════════

void FocusManager::setFocus(FocusNode* node) {
    if (current_focus == node) return;

    if (current_focus) {
        current_focus->has_focus = false;
        if (current_focus->on_focus_changed) {
            current_focus->on_focus_changed(false);
        }
    }

    current_focus = node;

    if (current_focus) {
        current_focus->has_focus = true;
        if (current_focus->on_focus_changed) {
            current_focus->on_focus_changed(true);
        }
    }
}

void FocusManager::clearFocus() {
    if (current_focus) {
        current_focus->has_focus = false;
        if (current_focus->on_focus_changed) {
            current_focus->on_focus_changed(false);
        }
        current_focus = nullptr;
    }
}

// ════════════════════════════════════════════════════════════════
// Focus Widget State
// ════════════════════════════════════════════════════════════════

class FocusState : public State {
private:
    std::shared_ptr<FocusNode> node_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const FocusWidget*>(widget());
        if (w->focus_node) {
            node_ = w->focus_node;
        } else {
            node_ = std::make_shared<FocusNode>();
        }

        node_->on_focus_changed = [this](bool foc) {
            auto* sw = static_cast<const FocusWidget*>(widget());
            if (sw->on_focus_change) sw->on_focus_change(foc);
            setState([] {});
        };

        if (w->autofocus) {
            node_->requestFocus();
        }
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const FocusWidget*>(widget());
        bool is_focused = node_ && node_->has_focus;

        std::optional<Border> focus_border;
        std::optional<BorderRadius> focus_radius;
        if (w->show_focus_ring && is_focused) {
            focus_border = Border(w->focus_ring_color, 2.0f);
            focus_radius = BorderRadius::circular(8.0f);
        }

        auto box = container({
            .border_radius = focus_radius,
            .border = focus_border,
            .child = w->child,
        });

        return gestureDetector({
            .child = box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                if (node_) node_->requestFocus();
            },
        });
    }
};

std::unique_ptr<State> FocusWidget::createState() {
    return std::make_unique<FocusState>();
}

class FocusScopeState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const FocusScopeWidget*>(widget());
        return w->child ? w->child : container();
    }
};

std::unique_ptr<State> FocusScopeWidget::createState() {
    return std::make_unique<FocusScopeState>();
}

} // namespace enki
