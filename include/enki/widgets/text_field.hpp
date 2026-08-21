#pragma once
/// @file text_field.hpp
/// @brief A complete interactive TextField widget.

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
    std::string hint_text;
    bool obscure_text = false;
    bool read_only = false;
    bool auto_focus = false;
    size_t max_lines = 1;
    
    Color cursor_color = 0xFF0078D7;
    Color selection_color = 0x640078D7;

    std::function<void(std::string)> on_changed = nullptr;
    std::function<void(std::string)> on_submitted = nullptr;
};

class TextField : public StatefulWidget {
public:
    std::shared_ptr<TextFieldController> controller;
    TextFieldProps options;

    TextField(std::shared_ptr<TextFieldController> ctrl, TextFieldProps opt = {})
        : controller(ctrl ? ctrl : std::make_shared<TextFieldController>()), options(std::move(opt)) {}

    TextField(Key key, std::shared_ptr<TextFieldController> ctrl, TextFieldProps opt)
        : StatefulWidget(std::move(key)), controller(ctrl ? std::move(ctrl) : std::make_shared<TextFieldController>()), options(std::move(opt)) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "TextField"; }
};

inline std::shared_ptr<TextField> textField(
    std::shared_ptr<TextFieldController> ctrl,
    TextFieldProps options = {}) {
    return std::make_shared<TextField>(std::move(ctrl), std::move(options));
}

inline std::shared_ptr<TextField> textField(TextFieldProps props) {
    return std::make_shared<TextField>(std::move(props.key), std::move(props.controller), std::move(props));
}

} // namespace enki
