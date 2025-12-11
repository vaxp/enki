#include "enki/widgets/tag_input.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include <algorithm>

#include "rich_input_focus.hpp"

namespace enki {

TagInputProps::operator WidgetPtr() const {
    return std::make_shared<TagInputWidget>(*this);
}

class TagInputState : public State {
public:
    std::vector<std::string> current_tags_;
    std::string input_text_ = "";

    SlotId text_input_conn_ = 0;
    SlotId key_down_conn_ = 0;

    void initState() override {
        State::initState();
        const auto* w = static_cast<const TagInputWidget*>(widget());
        current_tags_ = w->props.tags;

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
        const auto* w = static_cast<const TagInputWidget*>(widget());
        current_tags_ = w->props.tags;
    }

    void dispose() override {
        if (internal::g_rich_input_focus == this) internal::g_rich_input_focus = nullptr;
        if (Platform::instance()) {
            Platform::instance()->onTextInput().disconnect(text_input_conn_);
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    void addCurrentTag() {
        const auto* w = static_cast<const TagInputWidget*>(widget());
        std::string tag = input_text_;
        // Trim whitespace
        while (!tag.empty() && (tag.front() == ' ' || tag.front() == '\t')) tag.erase(tag.begin());
        while (!tag.empty() && (tag.back() == ' ' || tag.back() == '\t')) tag.pop_back();

        if (!tag.empty() && current_tags_.size() < static_cast<size_t>(w->props.max_tags)) {
            if (w->props.allow_duplicates || std::find(current_tags_.begin(), current_tags_.end(), tag) == current_tags_.end()) {
                current_tags_.push_back(tag);
                if (w->props.on_tags_changed) {
                    w->props.on_tags_changed(current_tags_);
                }
            }
        }
        input_text_.clear();
        setState([]() {});
    }

    void handleTextInput(std::string_view text) {
        for (char c : text) {
            if (c == ',' || c == '\n') {
                addCurrentTag();
            } else {
                input_text_ += c;
            }
        }
        setState([]() {});
    }

    void handleKeyDown(int key, int) {
        const auto* w = static_cast<const TagInputWidget*>(widget());
        constexpr int KEY_RETURN    = 0xff0d;
        constexpr int KEY_BACKSPACE = 0xff08;

        if (key == KEY_RETURN) {
            addCurrentTag();
        } else if (key == KEY_BACKSPACE) {
            if (input_text_.empty()) {
                if (!current_tags_.empty()) {
                    current_tags_.pop_back();
                    if (w->props.on_tags_changed) {
                        w->props.on_tags_changed(current_tags_);
                    }
                    setState([]() {});
                }
            } else {
                input_text_.pop_back();
                setState([]() {});
            }
        }
    }

    WidgetPtr build(BuildContext&) override {
        const auto* w = static_cast<const TagInputWidget*>(widget());
        std::vector<WidgetPtr> items;

        // 1. Existing Tag Chips
        for (size_t i = 0; i < current_tags_.size(); ++i) {
            std::string tag = current_tags_[i];

            auto tag_label = text(tag, {
                .color = w->props.chip_text_color,
                .font_size = 12.0f,
                .font_weight = FontWeight::Medium,
            });

            auto delete_btn = gestureDetector(GestureDetectorProps{
                .child = text(" ×", {
                    .color = w->props.delete_icon_color,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                }),
                .on_tap = [this, i]() {
                    if (i < current_tags_.size()) {
                        current_tags_.erase(current_tags_.begin() + i);
                        const auto* wid = static_cast<const TagInputWidget*>(widget());
                        if (wid->props.on_tags_changed) {
                            wid->props.on_tags_changed(current_tags_);
                        }
                        setState([]() {});
                    }
                },
            });

            auto chip_content = row(FlexboxProps{
                .align_items = Align::Center,
                .children = {tag_label, delete_btn},
            });

            auto chip = container(Container{
                .color = w->props.chip_background,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(w->props.chip_border_color, 1.0f),
                .padding = StyleInsets::symmetric(4.0f, 8.0f),
                .child = chip_content,
            });

            items.push_back(chip);
        }

        // 2. Active Text Input Area / Placeholder
        bool is_focused = (internal::g_rich_input_focus == this);
        std::string edit_display = input_text_.empty() ? (is_focused ? "│" : w->props.placeholder) : (input_text_ + (is_focused ? "│" : ""));
        Color text_col = input_text_.empty() && !is_focused ? 0x4DFFFFFF : 0xFFFFFFFF;

        auto input_field = container(Container{
            .padding = StyleInsets::symmetric(4.0f, 6.0f),
            .child = text(edit_display, {
                .color = text_col,
                .font_size = 12.5f,
            }),
        });

        items.push_back(input_field);

        auto tags_row = row(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = std::move(items),
        });

        Color border_col = is_focused ? 0xFF00E5FF : w->props.border_color;
        float border_w = is_focused ? 1.5f : 1.0f;

        auto outer_box = container(Container{
            .color = w->props.container_background,
            .border_radius = BorderRadius::circular(w->props.border_radius),
            .border = Border(border_col, border_w),
            .padding = StyleInsets::symmetric(6.0f, 10.0f),
            .child = tags_row,
        });

        return gestureDetector(GestureDetectorProps{
            .child = outer_box,
            .on_tap = [this]() {
                internal::g_rich_input_focus = this;
                setState([]() {});
            },
        });
    }
};

std::unique_ptr<State> TagInputWidget::createState() {
    return std::make_unique<TagInputState>();
}

} // namespace enki
