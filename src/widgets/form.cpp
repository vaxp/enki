/// @file form.cpp
/// @brief Implementation of Advanced Form & FormField validation suite for ENKI Framework.

#include "enki/widgets/form.hpp"
#include "enki/widgets/checkbox.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/tree/build_context.hpp"

#include <iostream>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Form Widget State
// ════════════════════════════════════════════════════════════════

class FormWidgetState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const FormWidget*>(widget());
        return w->child;
    }
};

std::unique_ptr<State> FormWidget::createState() {
    return std::make_unique<FormWidgetState>();
}

// ════════════════════════════════════════════════════════════════
// TextFormField State
// ════════════════════════════════════════════════════════════════

class TextFormFieldState : public State, public FormFieldStateBase {
private:
    std::shared_ptr<TextFieldController> controller_;
    std::string error_text_;
    bool is_touched_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        const auto& opts = w->props;

        if (opts.controller) {
            controller_ = opts.controller;
        } else {
            controller_ = std::make_shared<TextFieldController>(opts.initial_value);
        }

        if (opts.form_state) {
            opts.form_state->registerField(this);
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        if (w && w->props.controller) {
            controller_ = w->props.controller;
        }
    }

    void dispose() override {
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        if (w && w->props.form_state) {
            w->props.form_state->unregisterField(this);
        }
        State::dispose();
    }

    bool validate() override {
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        if (w && w->props.validator) {
            auto res = w->props.validator(controller_->text);
            std::string prev_err = error_text_;
            error_text_ = res.value_or("");
            if (prev_err != error_text_) {
                setState([] {});
            }
            return error_text_.empty();
        }
        return true;
    }

    void save() override {
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        if (w && w->props.on_saved) {
            w->props.on_saved(controller_->text);
        }
    }

    void reset() override {
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        if (w) {
            controller_->text = w->props.initial_value;
            controller_->clearSelection();
            error_text_.clear();
            is_touched_ = false;
            setState([] {});
        }
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const TextFormFieldWidget*>(widget());
        const auto& opts = w->props;

        std::vector<WidgetPtr> col_items;

        // 1. Label Row (with required asterisk)
        if (!opts.label.empty()) {
            auto lbl = text({
                .text = opts.label,
                .color = 0xFFE2E8F0,
                .font_size = 12.5f,
                .font_weight = FontWeight::Bold,
            });

            std::vector<WidgetPtr> lbl_items = {lbl};
            if (opts.required) {
                auto ast = text({
                    .text = "*",
                    .color = 0xFFEF4444,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold,
                });
                lbl_items.push_back(ast);
            }

            auto lbl_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = std::move(lbl_items),
            });
            col_items.push_back(lbl_row);
        }

        // 2. Main TextField Input
        TextFieldProps tf_opts;
        tf_opts.hint_text = opts.hint;
        tf_opts.obscure_text = opts.obscure_text;
        tf_opts.on_changed = [this](std::string val) {
            is_touched_ = true;
            auto* cur_w = static_cast<const TextFormFieldWidget*>(widget());
            if (cur_w && cur_w->props.on_changed) {
                cur_w->props.on_changed(val);
            }
            if (!error_text_.empty()) {
                validate();
            }
        };

        auto tf = std::make_shared<TextFieldWidget>(controller_, tf_opts);

        auto tf_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .border = !error_text_.empty() ? Border(0xFFEF4444, 1.5f) : Border(0xFF334155, 1.0f),
            .width = StyleValue::point(opts.width),
            .padding = StyleInsets::symmetric(4.0f, 10.0f),
            .child = tf,
        });

        col_items.push_back(tf_box);

        // 3. Error Banner or Helper Text
        if (!error_text_.empty()) {
            auto err_icon = text({
                .text = "⚠️",
                .font_size = 11.0f,
            });

            auto err_msg = text({
                .text = error_text_,
                .color = 0xFFEF4444,
                .font_size = 11.5f,
            });

            auto err_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = {err_icon, err_msg},
            });
            col_items.push_back(err_row);
        } else if (!opts.helper_text.empty()) {
            auto hlp = text({
                .text = opts.helper_text,
                .color = 0xFF64748B,
                .font_size = 11.0f,
            });
            col_items.push_back(hlp);
        }

        return column({
            .gap = StyleValue::point(5.0f),
            .children = std::move(col_items),
        });
    }
};

std::unique_ptr<State> TextFormFieldWidget::createState() {
    return std::make_unique<TextFormFieldState>();
}

// ════════════════════════════════════════════════════════════════
// CheckboxFormField State
// ════════════════════════════════════════════════════════════════

class CheckboxFormFieldState : public State, public FormFieldStateBase {
private:
    bool is_checked_ = false;
    std::string error_text_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        is_checked_ = w->props.initial_value;

        if (w->props.form_state) {
            w->props.form_state->registerField(this);
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        if (w) {
            is_checked_ = w->props.initial_value;
        }
    }

    void dispose() override {
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        if (w && w->props.form_state) {
            w->props.form_state->unregisterField(this);
        }
        State::dispose();
    }

    bool validate() override {
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        if (w && w->props.validator) {
            auto res = w->props.validator(is_checked_);
            std::string prev_err = error_text_;
            error_text_ = res.value_or("");
            if (prev_err != error_text_) {
                setState([] {});
            }
            return error_text_.empty();
        }
        return true;
    }

    void save() override {
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        if (w && w->props.on_saved) {
            w->props.on_saved(is_checked_);
        }
    }

    void reset() override {
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        if (w) {
            is_checked_ = w->props.initial_value;
            error_text_.clear();
            setState([] {});
        }
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const CheckboxFormFieldWidget*>(widget());
        const auto& opts = w->props;

        auto cb = (WidgetPtr)Checkbox {
            .value = is_checked_,
            .on_changed = [this, opts](bool val) {
                is_checked_ = val;
                if (opts.on_changed) opts.on_changed(val);
                if (!error_text_.empty()) {
                    validate();
                }
                setState([] {});
            }
        };

        auto lbl = text({
            .text = opts.label,
            .color = 0xFFE2E8F0,
            .font_size = 12.5f,
        });

        std::vector<WidgetPtr> row_items = {cb, lbl};
        if (opts.required) {
            auto ast = text({
                .text = "*",
                .color = 0xFFEF4444,
                .font_size = 12.5f,
                .font_weight = FontWeight::Bold,
            });
            row_items.push_back(ast);
        }

        auto cb_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = std::move(row_items),
        });

        std::vector<WidgetPtr> col_items = {cb_row};

        if (!error_text_.empty()) {
            auto err_icon = text({
                .text = "⚠️",
                .font_size = 11.0f,
            });

            auto err_msg = text({
                .text = error_text_,
                .color = 0xFFEF4444,
                .font_size = 11.5f,
            });

            auto err_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = {err_icon, err_msg},
            });
            col_items.push_back(err_row);
        }

        return column({
            .gap = StyleValue::point(4.0f),
            .children = std::move(col_items),
        });
    }
};

std::unique_ptr<State> CheckboxFormFieldWidget::createState() {
    return std::make_unique<CheckboxFormFieldState>();
}

} // namespace enki
