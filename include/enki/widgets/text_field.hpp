#pragma once
/// @file text_field.hpp
/// @brief A complete interactive TextField widget.
///
/// Features:
///   - Text editing with cursor, selection, UTF-8 & Arabic input support.
///   - Read-only and obscure_text (password) modes.
///   - Clipboard copy/paste/cut integration.
///   - Auto-focus, cursor blinking, on_changed and on_submitted callbacks.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <functional>
#include <memory>

namespace enki {

struct TextFieldController {
    std::string text;
    size_t selection_start = 0;
    size_t selection_end = 0;

    TextFieldController(std::string initial_text = "") : text(std::move(initial_text)) {}

    void clearSelection() {
        selection_end = selection_start;
    }
    void selectAll() {
        selection_start = 0;
        selection_end = text.length();
    }
    bool hasSelection() const {
        return selection_start != selection_end;
    }
};

struct TextFieldProps {
    Key key = Key::none();
    std::shared_ptr<TextFieldController> controller = nullptr;

    TextStyle style;
    std::string hint_text = "";
    bool obscure_text = false;
    bool read_only = false;
    bool auto_focus = false;
    size_t max_lines = 1;
    
    Color cursor_color = 0xFF0078D7;
    Color selection_color = 0x640078D7;

    std::function<void(std::string)> on_changed = nullptr;
    std::function<void(std::string)> on_submitted = nullptr;
};

class TextFieldWidget : public StatefulWidget {
public:
    std::shared_ptr<TextFieldController> controller;
    TextFieldProps options;

    TextFieldWidget()
        : controller(std::make_shared<TextFieldController>()) {}
    explicit TextFieldWidget(TextFieldProps opt)
        : controller(opt.controller ? opt.controller : std::make_shared<TextFieldController>()), options(std::move(opt)) {}
    TextFieldWidget(Key key, TextFieldProps opt)
        : StatefulWidget(std::move(key)), controller(opt.controller ? opt.controller : std::make_shared<TextFieldController>()), options(std::move(opt)) {}
    TextFieldWidget(std::shared_ptr<TextFieldController> ctrl, TextFieldProps opt = {})
        : controller(ctrl ? ctrl : std::make_shared<TextFieldController>()), options(std::move(opt)) {}
    TextFieldWidget(Key key, std::shared_ptr<TextFieldController> ctrl, TextFieldProps opt)
        : StatefulWidget(std::move(key)), controller(ctrl ? std::move(ctrl) : std::make_shared<TextFieldController>()), options(std::move(opt)) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "TextField"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative TextField Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct TextField {
    Key key = Key::none();
    std::shared_ptr<TextFieldController> controller = nullptr;

    TextStyle style = {};
    std::string hint_text = "";
    std::string hint = "";
    bool obscure_text = false;
    bool read_only = false;
    bool auto_focus = false;
    size_t max_lines = 1;
    
    Color cursor_color = 0xFF0078D7;
    Color selection_color = 0x640078D7;

    std::function<void(std::string)> on_changed = nullptr;
    std::function<void(std::string)> on_submitted = nullptr;

    operator WidgetPtr() const {
        TextFieldProps p;
        p.key = key;
        p.controller = controller;
        p.style = style;
        p.hint_text = !hint.empty() ? hint : hint_text;
        p.obscure_text = obscure_text;
        p.read_only = read_only;
        p.auto_focus = auto_focus;
        p.max_lines = max_lines;
        p.cursor_color = cursor_color;
        p.selection_color = selection_color;
        p.on_changed = on_changed;
        p.on_submitted = on_submitted;
        return std::make_shared<TextFieldWidget>(key, std::move(p));
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<TextFieldWidget> textField(
    std::shared_ptr<TextFieldController> ctrl,
    TextFieldProps options = {}) {
    options.controller = ctrl;
    return std::make_shared<TextFieldWidget>(std::move(ctrl), std::move(options));
}

inline std::shared_ptr<TextFieldWidget> textField(TextFieldProps props) {
    auto k = props.key;
    auto ctrl = props.controller;
    return std::make_shared<TextFieldWidget>(std::move(k), std::move(ctrl), std::move(props));
}

} // namespace enki
