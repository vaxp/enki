#include "enki/widgets/pin_field.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/platform/platform.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include <chrono>
#include <algorithm>
#include <cctype>

#include "rich_input_focus.hpp"

namespace enki {

PinFieldProps::operator WidgetPtr() const {
    return std::make_shared<PinFieldWidget>(*this);
}

class PinFieldState : public State {
public:
    std::string pin_ = "";
    int last_unmasked_idx_ = -1;
    std::chrono::steady_clock::time_point last_typed_time_;
    std::unique_ptr<Ticker> mask_ticker_;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;

    void initState() override {
        State::initState();
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        pin_ = w->props.initial_value;

        if (w->props.auto_focus) {
            internal::g_rich_input_focus = this;
        }

        mask_ticker_ = createTicker([this]() {
            checkMaskDelay();
        });

        if (Platform::instance()) {
            text_input_conn_ = Platform::instance()->onTextInput().connect([this](std::string_view text) {
                if (internal::g_rich_input_focus != this) return;
                handleTextInput(text);
            });

            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                if (internal::g_rich_input_focus != this) return;
                handleKeyDown(key, mods);
            });
        }
    }

    void didUpdateWidget(const Widget& oldWidget) override {
        State::didUpdateWidget(oldWidget);
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        if (!w->props.initial_value.empty() && pin_.empty()) {
            pin_ = w->props.initial_value;
        }
    }

    void dispose() override {
        if (internal::g_rich_input_focus == this) internal::g_rich_input_focus = nullptr;
        if (mask_ticker_) {
            mask_ticker_->stop();
            mask_ticker_.reset();
        }
        if (Platform::instance()) {
            Platform::instance()->onTextInput().disconnect(text_input_conn_);
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    void checkMaskDelay() {
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        if (last_unmasked_idx_ < 0) {
            if (mask_ticker_) mask_ticker_->stop();
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_typed_time_).count();
        if (ms >= w->props.obscure_delay_ms) {
            last_unmasked_idx_ = -1;
            if (mask_ticker_) mask_ticker_->stop();
            setState([]() {});
        }
    }

    void handleTextInput(std::string_view text) {
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        if (text.empty()) return;

        for (char c : text) {
            if (pin_.length() >= static_cast<size_t>(w->props.length)) break;

            if (w->props.numeric_only && !std::isdigit(static_cast<unsigned char>(c))) {
                continue;
            }

            pin_ += c;
            last_unmasked_idx_ = static_cast<int>(pin_.length()) - 1;
            last_typed_time_ = std::chrono::steady_clock::now();

            if (w->props.obscure_delay_ms > 0) {
                if (mask_ticker_) mask_ticker_->start();
            } else {
                last_unmasked_idx_ = -1;
            }
        }

        if (w->props.on_changed) w->props.on_changed(pin_);

        if (pin_.length() == static_cast<size_t>(w->props.length)) {
            if (w->props.on_completed) w->props.on_completed(pin_);
        }

        setState([]() {});
    }

    void handleKeyDown(int key, int) {
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        constexpr int KEY_BACKSPACE = 0xff08;

        if (key == KEY_BACKSPACE) {
            if (!pin_.empty()) {
                pin_.pop_back();
                last_unmasked_idx_ = -1;
                if (w->props.on_changed) w->props.on_changed(pin_);
                setState([]() {});
            }
        }
    }

    WidgetPtr build(BuildContext&) override {
        const auto* w = static_cast<const PinFieldWidget*>(widget());
        std::vector<WidgetPtr> cells;

        bool is_focused = (internal::g_rich_input_focus == this);

        for (int i = 0; i < w->props.length; ++i) {
            bool is_current = is_focused && (static_cast<int>(pin_.length()) == i);
            bool is_filled = (i < static_cast<int>(pin_.length()));

            std::string disp;
            Color dot_color = w->props.filled_dot_color;

            if (is_filled) {
                if (i == last_unmasked_idx_) {
                    disp = std::string(1, pin_[i]);
                } else {
                    disp = w->props.mask_char;
                }
            } else {
                disp = is_current ? "│" : " ";
                dot_color = 0x33FFFFFF;
            }

            Color border_col = w->props.has_error ? w->props.error_color
                             : (is_current ? w->props.active_border_color : w->props.inactive_border_color);
            float border_w = is_current ? 2.0f : 1.0f;

            auto txt = text(disp, {
                .color = dot_color,
                .font_size = (disp == w->props.mask_char) ? 22.0f : 18.0f,
                .font_weight = FontWeight::Bold,
            });

            auto cell = container({
                .color = w->props.box_background,
                .border_radius = BorderRadius::circular(w->props.border_radius),
                .border = Border(border_col, border_w),
                .align = Alignment::Center,
                .width = StyleValue::point(w->props.box_size),
                .height = StyleValue::point(w->props.box_size),
                .child = txt,
            });

            cells.push_back(gestureDetector(GestureDetectorProps{
                .child = cell,
                .on_tap = [this]() {
                    internal::g_rich_input_focus = this;
                    setState([]() {});
                },
            }));
        }

        return row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(w->props.gap),
            .children = std::move(cells),
        });
    }
};

std::unique_ptr<State> PinFieldWidget::createState() {
    return std::make_unique<PinFieldState>();
}

} // namespace enki
