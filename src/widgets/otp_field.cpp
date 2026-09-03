#include "enki/widgets/otp_field.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include <algorithm>
#include <cctype>

#include "rich_input_focus.hpp"

namespace enki {

OTPFieldProps::operator WidgetPtr() const {
    return std::make_shared<OTPFieldWidget>(*this);
}

class OTPFieldState : public State {
public:
    std::vector<char> digits_;
    int focus_index_ = 0;

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;

    void initState() override {
        State::initState();
        const auto* w = static_cast<const OTPFieldWidget*>(widget());
        digits_.resize(w->props.length, ' ');

        for (size_t i = 0; i < w->props.initial_value.length() && i < digits_.size(); ++i) {
            digits_[i] = w->props.initial_value[i];
        }

        if (w->props.auto_focus) {
            internal::g_rich_input_focus = this;
        }

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
        const auto* w = static_cast<const OTPFieldWidget*>(widget());
        if (!w->props.initial_value.empty() && getFullCode().empty()) {
            for (size_t i = 0; i < w->props.initial_value.length() && i < digits_.size(); ++i) {
                digits_[i] = w->props.initial_value[i];
            }
        }
    }

    void dispose() override {
        if (internal::g_rich_input_focus == this) internal::g_rich_input_focus = nullptr;
        if (Platform::instance()) {
            Platform::instance()->onTextInput().disconnect(text_input_conn_);
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    std::string getFullCode() const {
        std::string s;
        for (char c : digits_) {
            if (c != ' ') s += c;
        }
        return s;
    }

    void handleTextInput(std::string_view text) {
        const auto* w = static_cast<const OTPFieldWidget*>(widget());
        if (text.empty()) return;

        // Multiple characters (Paste distribution)
        if (text.length() > 1) {
            size_t idx = 0;
            for (char c : text) {
                if (std::isalnum(static_cast<unsigned char>(c)) && idx < digits_.size()) {
                    digits_[idx++] = c;
                }
            }
            focus_index_ = std::min(static_cast<int>(digits_.size()) - 1, static_cast<int>(idx));
        } else {
            // Single character typed
            char c = text[0];
            if (std::isalnum(static_cast<unsigned char>(c))) {
                if (focus_index_ >= 0 && focus_index_ < static_cast<int>(digits_.size())) {
                    digits_[focus_index_] = c;
                    if (focus_index_ < static_cast<int>(digits_.size()) - 1) {
                        focus_index_++;
                    }
                }
            }
        }

        std::string code = getFullCode();
        if (w->props.on_changed) w->props.on_changed(code);

        if (code.length() == static_cast<size_t>(w->props.length)) {
            if (w->props.on_completed) w->props.on_completed(code);
        }

        setState([]() {});
    }

    void handleKeyDown(int key, int) {
        const auto* w = static_cast<const OTPFieldWidget*>(widget());
        constexpr int KEY_BACKSPACE = 0xff08;
        constexpr int KEY_LEFT      = 0xff51;
        constexpr int KEY_RIGHT     = 0xff53;

        if (key == KEY_BACKSPACE) {
            if (focus_index_ >= 0 && focus_index_ < static_cast<int>(digits_.size())) {
                if (digits_[focus_index_] != ' ') {
                    digits_[focus_index_] = ' ';
                } else if (focus_index_ > 0) {
                    focus_index_--;
                    digits_[focus_index_] = ' ';
                }
                if (w->props.on_changed) w->props.on_changed(getFullCode());
                setState([]() {});
            }
        } else if (key == KEY_LEFT) {
            if (focus_index_ > 0) {
                focus_index_--;
                setState([]() {});
            }
        } else if (key == KEY_RIGHT) {
            if (focus_index_ < static_cast<int>(digits_.size()) - 1) {
                focus_index_++;
                setState([]() {});
            }
        }
    }

    WidgetPtr build(BuildContext&) override {
        const auto* w = static_cast<const OTPFieldWidget*>(widget());
        std::vector<WidgetPtr> boxes;

        bool is_focused = (internal::g_rich_input_focus == this);

        for (int i = 0; i < w->props.length; ++i) {
            bool is_current = is_focused && (focus_index_ == i);
            char c = (i < static_cast<int>(digits_.size())) ? digits_[i] : ' ';

            std::string disp_text;
            if (c == ' ') {
                disp_text = is_current ? "│" : "·";
            } else if (w->props.is_obscured) {
                disp_text = "●";
            } else {
                disp_text = std::string(1, c);
            }

            Color border_col = w->props.has_error ? w->props.error_color
                             : (is_current ? w->props.active_border_color : w->props.inactive_border_color);
            float border_w = is_current ? 2.0f : 1.0f;

            auto txt = text(disp_text, {
                .color = (c == ' ' && !is_current) ? 0x4DFFFFFF : w->props.text_color,
                .font_size = 20.0f,
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

            boxes.push_back(gestureDetector(GestureDetectorProps{
                .child = cell,
                .on_tap = [this, i]() {
                    internal::g_rich_input_focus = this;
                    focus_index_ = i;
                    setState([]() {});
                },
            }));
        }

        return row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(w->props.gap),
            .children = std::move(boxes),
        });
    }
};

std::unique_ptr<State> OTPFieldWidget::createState() {
    return std::make_unique<OTPFieldState>();
}

} // namespace enki
