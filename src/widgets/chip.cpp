/// @file chip.cpp
/// @brief Implementation of Advanced Chip & ChipGroup widgets for ENKI Framework.

#include "enki/widgets/chip.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <iostream>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Chip State
// ════════════════════════════════════════════════════════════════

class ChipState : public State {
private:
    bool is_selected_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const ChipWidget*>(widget());
        is_selected_ = w->options.selected;
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        auto* w = static_cast<const ChipWidget*>(widget());
        is_selected_ = w->options.selected;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ChipWidget*>(widget());
        const auto& opts = w->options;

        // Size attributes
        float font_size = 12.0f;
        float h_pad = 12.0f;
        float v_pad = 5.0f;
        float border_radius = 16.0f;

        if (opts.size == ChipSize::Small) {
            font_size = 10.5f;
            h_pad = 8.0f;
            v_pad = 2.5f;
            border_radius = 12.0f;
        } else if (opts.size == ChipSize::Large) {
            font_size = 13.0f;
            h_pad = 16.0f;
            v_pad = 7.0f;
            border_radius = 20.0f;
        }

        std::vector<WidgetPtr> items;

        // 1. Leading Component
        if (opts.type == ChipType::Filter && is_selected_) {
            auto check = text({
                .text = "✓",
                .color = 0xFFFFFFFF,
                .font_size = font_size,
                .font_weight = FontWeight::Bold,
            });
            items.push_back(check);
        } else if (opts.type == ChipType::Status && opts.pulsing_dot) {
            auto dot = container({
                .color = opts.status_color,
                .border_radius = BorderRadius::circular(4.0f),
                .width = StyleValue::point(8.0f),
                .height = StyleValue::point(8.0f),
            });
            items.push_back(dot);
        } else if (!opts.avatar_icon.empty()) {
            auto av = text({
                .text = opts.avatar_icon,
                .font_size = font_size + 1.0f,
            });
            items.push_back(av);
        } else if (opts.leading) {
            items.push_back(opts.leading);
        }

        // 2. Chip Label
        Color lbl_color = is_selected_ ? 0xFFFFFFFF : opts.text_color;
        FontWeight lbl_weight = is_selected_ ? FontWeight::Bold : FontWeight::Normal;
        auto lbl_txt = text({
            .text = opts.label,
            .color = lbl_color,
            .font_size = font_size,
            .font_weight = lbl_weight,
        });
        items.push_back(lbl_txt);

        // 3. Trailing Component
        if (opts.deletable) {
            auto del_txt = text({
                .text = "✕",
                .color = 0xFF94A3B8,
                .font_size = font_size - 1.5f,
                .font_weight = FontWeight::Bold,
            });

            auto del_box = container({
                .border_radius = BorderRadius::circular(8.0f),
                .padding = StyleInsets::symmetric(1.0f, 3.0f),
                .child = del_txt,
            });

            auto del_gd = gestureDetector({
                .child = del_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [opts](const TapUpDetails&) {
                    if (opts.on_deleted) opts.on_deleted();
                },
            });
            items.push_back(del_gd);
        } else if (opts.trailing) {
            items.push_back(opts.trailing);
        }

        auto chip_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = std::move(items),
        });

        // Styling based on state and variant
        Color chip_bg;
        std::optional<Border> chip_border;
        std::vector<BoxShadow> chip_shadows;

        if (!opts.enabled) {
            chip_bg = 0x33334155;
        } else if (is_selected_) {
            chip_bg = opts.selected_color;
        } else if (opts.variant == ChipVariant::Filled) {
            chip_bg = opts.background_color;
            chip_border = Border(opts.border_color, 1.0f);
        } else if (opts.variant == ChipVariant::Outlined) {
            chip_bg = 0x00000000;
            chip_border = Border(opts.border_color, 1.2f);
        } else {
            // Elevated
            chip_bg = opts.background_color;
            chip_border = Border(opts.border_color, 1.0f);
            chip_shadows.push_back(BoxShadow(0x55000000, {0.0f, 2.0f}, 6.0f));
        }

        auto chip_box = container({
            .color = chip_bg,
            .border_radius = BorderRadius::circular(border_radius),
            .border = chip_border,
            .box_shadow = std::move(chip_shadows),
            .padding = StyleInsets::symmetric(v_pad, h_pad),
            .child = chip_row,
        });

        if (!opts.enabled) {
            return chip_box;
        }

        return gestureDetector({
            .child = chip_box,
            .cursor_type = (opts.type == ChipType::Action || opts.type == ChipType::Filter || opts.type == ChipType::Choice)
                               ? SystemCursor::Pointer
                               : SystemCursor::Default,
            .on_tap_up = [this, opts](const TapUpDetails&) {
                if (opts.type == ChipType::Filter) {
                    is_selected_ = !is_selected_;
                    if (opts.on_selected) opts.on_selected(is_selected_);
                    setState([] {});
                } else if (opts.type == ChipType::Choice) {
                    is_selected_ = !is_selected_;
                    if (opts.on_selected) opts.on_selected(is_selected_);
                    setState([] {});
                } else if (opts.type == ChipType::Action) {
                    if (opts.on_tap) opts.on_tap();
                }
            },
        });
    }
};

std::unique_ptr<State> ChipWidget::createState() {
    return std::make_unique<ChipState>();
}

// ════════════════════════════════════════════════════════════════
// ChipGroup State
// ════════════════════════════════════════════════════════════════

class ChipGroupState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ChipGroupWidget*>(widget());

        std::vector<WidgetPtr> chip_items;
        for (const auto& c : w->chips) {
            if (c) chip_items.push_back(c);
        }

        return wrap({
            .align_items = Align::Center,
            .gap = StyleValue::point(w->options.gap),
            .children = std::move(chip_items),
        });
    }
};

std::unique_ptr<State> ChipGroupWidget::createState() {
    return std::make_unique<ChipGroupState>();
}

} // namespace enki
