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
            auto dot = container();
            dot->color(opts.status_color).width(8.0f).height(8.0f).borderRadius(4.0f);
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

            auto del_box = container(del_txt);
            del_box->borderRadius(8.0f).paddingSymmetric(1.0f, 3.0f);

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

        auto chip_row = row(items);
        chip_row->gap(StyleValue::point(6.0f)).alignItems(Align::Center);

        auto chip_box = container(chip_row);
        chip_box->paddingSymmetric(v_pad, h_pad).borderRadius(border_radius);

        // Styling based on state and variant
        if (is_selected_) {
            chip_box->color(opts.selected_color);
        } else if (opts.variant == ChipVariant::Filled) {
            chip_box->color(opts.background_color).border(opts.border_color, 1.0f);
        } else if (opts.variant == ChipVariant::Outlined) {
            chip_box->color(0x00000000).border(opts.border_color, 1.2f);
        } else {
            // Elevated
            chip_box->color(opts.background_color)
                    .border(opts.border_color, 1.0f)
                    .shadow(BoxShadow(0x55000000, {0.0f, 2.0f}, 6.0f));
        }

        if (!opts.enabled) {
            chip_box->color(0x33334155);
            return chip_box;
        }

        return gestureDetector({
            .child = chip_box,
            .cursor_type = SystemCursor::Pointer,
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

        auto wr = wrap(chip_items);
        wr->gap(StyleValue::point(w->options.gap)).alignItems(Align::Center);
        return wr;
    }
};

std::unique_ptr<State> ChipGroupWidget::createState() {
    return std::make_unique<ChipGroupState>();
}

} // namespace enki
