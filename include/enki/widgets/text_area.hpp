#pragma once
/// @file text_area.hpp
/// @brief Advanced multi-line TextArea widget with line numbers, undo/redo, scrolling, and counters.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <sstream>
#include <algorithm>

namespace enki {

/// @brief Controller for managing TextArea state, selection, cursor, and undo/redo history.
struct TextAreaController {
    std::string text;
    size_t selection_start = 0;
    size_t selection_end = 0;

    std::vector<std::string> undo_stack;
    std::vector<std::string> redo_stack;
    static constexpr size_t kMaxHistory = 100;

    TextAreaController(std::string initial_text = "")
        : text(std::move(initial_text)) {}

    void setText(std::string new_text) {
        if (new_text != text) {
            pushUndo(text);
            text = std::move(new_text);
            selection_start = std::min(selection_start, text.length());
            selection_end = selection_start;
        }
    }

    void clearSelection() {
        selection_end = selection_start;
    }

    void selectAll() {
        selection_start = 0;
        selection_end = text.length();
    }

    [[nodiscard]] bool hasSelection() const {
        return selection_start != selection_end;
    }

    [[nodiscard]] std::string getSelectedText() const {
        if (!hasSelection()) return "";
        size_t start = std::min(selection_start, selection_end);
        size_t end = std::max(selection_start, selection_end);
        return text.substr(start, end - start);
    }

    void pushUndo(const std::string& prev) {
        undo_stack.push_back(prev);
        if (undo_stack.size() > kMaxHistory) {
            undo_stack.erase(undo_stack.begin());
        }
        redo_stack.clear();
    }

    [[nodiscard]] bool canUndo() const { return !undo_stack.empty(); }
    [[nodiscard]] bool canRedo() const { return !redo_stack.empty(); }

    bool undo() {
        if (undo_stack.empty()) return false;
        redo_stack.push_back(text);
        text = undo_stack.back();
        undo_stack.pop_back();
        selection_start = std::min(selection_start, text.length());
        selection_end = selection_start;
        return true;
    }

    bool redo() {
        if (redo_stack.empty()) return false;
        undo_stack.push_back(text);
        text = redo_stack.back();
        redo_stack.pop_back();
        selection_start = std::min(selection_start, text.length());
        selection_end = selection_start;
        return true;
    }

    [[nodiscard]] size_t getLineCount() const {
        if (text.empty()) return 1;
        size_t count = 1;
        for (char c : text) {
            if (c == '\n') ++count;
        }
        return count;
    }

    [[nodiscard]] size_t getWordCount() const {
        if (text.empty()) return 0;
        std::istringstream iss(text);
        std::string word;
        size_t count = 0;
        while (iss >> word) ++count;
        return count;
    }

    void getCursorPosition(size_t& row, size_t& col) const {
        row = 1;
        col = 1;
        size_t limit = std::min(selection_start, text.length());
        for (size_t i = 0; i < limit; ++i) {
            if (text[i] == '\n') {
                ++row;
                col = 1;
            } else {
                ++col;
            }
        }
    }

    bool copyToClipboard();
    bool cutToClipboard();
    bool pasteFromClipboard();
};

/// Configuration styling and functional options for TextArea
struct TextAreaOptions {
    TextStyle style;
    std::string hint_text = "";
    bool read_only = false;
    bool auto_focus = false;

    size_t min_lines = 4;
    size_t max_lines = 12;
    bool auto_grow = false;

    bool show_line_numbers = false;
    bool show_counter = false;
    size_t max_characters = 0;

    Color cursor_color       = 0xFF38BDF8;
    Color selection_color    = 0x6438BDF8;
    Color background_color   = 0xFF1E293B;
    Color border_color       = 0xFF334155;
    Color focus_border_color = 0xFF38BDF8;

    Color line_number_color  = 0xFF64748B;
    Color line_number_bg     = 0xFF0F172A;

    float border_radius      = 8.0f;
    EdgeInsets padding       = EdgeInsets::all(10.0f);

    std::function<void(std::string)> on_changed;
    std::function<void(std::string)> on_submitted;
    std::function<void(size_t row, size_t col)> on_cursor_moved;
};

/// @brief Advanced multi-line TextArea widget.
class TextArea : public StatefulWidget {
public:
    std::shared_ptr<TextAreaController> controller;
    TextAreaOptions options;

    explicit TextArea(std::shared_ptr<TextAreaController> ctrl = nullptr, TextAreaOptions opt = {})
        : controller(ctrl ? ctrl : std::make_shared<TextAreaController>()),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TextArea"; }
};

inline WidgetPtr textArea(
    std::shared_ptr<TextAreaController> controller = nullptr,
    TextAreaOptions options = {}) {
    return std::make_shared<TextArea>(std::move(controller), std::move(options));
}

} // namespace enki
