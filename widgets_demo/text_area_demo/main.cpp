/// @file main.cpp
/// @brief ENKI Advanced TextArea Widget Interactive Demo.
/// Demonstrates multi-line text editing, line numbers, clipboard copy/cut/paste, word count, undo/redo, and scrolling.

#include "enki/app/app.hpp"
#include "enki/widgets/text_area.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/card.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ── Small Toolbar Action Button ───────────────────────────────────

class ActionBtn : public StatefulWidget {
public:
    std::string label;
    std::function<void()> on_press;
    Color bg_color;

    ActionBtn(std::string label, std::function<void()> on_press, Color bg = 0xFF334155)
        : label(std::move(label)), on_press(std::move(on_press)), bg_color(bg) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ActionBtn"; }
};

class ActionBtnState : public State {
private:
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        auto* btn = static_cast<const ActionBtn*>(widget());

        auto txt = text(btn->label, {
            .color = 0xFFFFFFFF,
            .font_size = 11.5f,
            .font_weight = FontWeight::Bold,
        });

        auto box = container({
            .color = hovered_ ? 0xFF475569 : btn->bg_color,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::symmetric(5.0f, 10.0f),
            .child = txt
        });

        return gestureDetector({
            .child = box,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type       = SystemCursor::Pointer,
            .on_tap = [btn] { if (btn->on_press) btn->on_press(); },
            .on_hover_enter = [this](const PointerEvent&) { setState([this] { hovered_ = true; }); },
            .on_hover_exit  = [this](const PointerEvent&) { setState([this] { hovered_ = false; }); },
        });
    }
};

std::unique_ptr<State> ActionBtn::createState() {
    return std::make_unique<ActionBtnState>();
}

// ── Main Demo App State ───────────────────────────────────────────

class TextAreaDemoState : public State {
private:
    std::shared_ptr<TextAreaController> code_controller_;
    std::shared_ptr<TextAreaController> notes_controller_;
    std::string cursor_info_ = "Ln 1, Col 1";

public:
    void initState() override {
        State::initState();

        std::string initial_code =
            "// ENKI High Performance Reactive GUI Framework\n"
            "#include <enki/app/app.hpp>\n"
            "#include <enki/widgets/flexbox.hpp>\n"
            "#include <iostream>\n\n"
            "int main() {\n"
            "    std::cout << \"Hello ENKI Engine!\" << std::endl;\n"
            "    // High performance multi-line editor with Clipboard\n"
            "    // Supports Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A, Ctrl+Z, Ctrl+Y\n"
            "    return 0;\n"
            "}\n";

        code_controller_ = std::make_shared<TextAreaController>(initial_code);

        std::string initial_notes =
            "Welcome to the ENKI Advanced TextArea!\n"
            "Features:\n"
            "• True multi-line text layout via Skia & SkParagraph\n"
            "• Full Clipboard Support (Copy, Cut, Paste, Select All)\n"
            "• Undo / Redo history stack (Ctrl+Z / Ctrl+Y)\n"
            "• Optional line numbers gutter on the left\n"
            "• Word count & character limits\n"
            "• Smooth scrolling & cursor tracking\n"
            "• Beautiful rounded dark UI theme\n";

        notes_controller_ = std::make_shared<TextAreaController>(initial_notes);
    }

    WidgetPtr build(BuildContext&) override {
        // Title Header
        auto title = text("Advanced TextArea Widget Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Multi-line text editor with line numbers, clipboard (Copy/Cut/Paste), undo/redo, and word count", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .children = {title, sub}
        });

        // ── 1. Code Editor Section (With Line Numbers) ─────────────────
        auto code_header = text("1. Code / Script Editor (Line Numbers & Monospace)", {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        // Code Editor Toolbar
        auto btn_copy1 = std::make_shared<ActionBtn>("📋 Copy", [this]() {
            code_controller_->copyToClipboard();
            setState([this] {});
        });

        auto btn_cut1 = std::make_shared<ActionBtn>("✂️ Cut", [this]() {
            code_controller_->cutToClipboard();
            setState([this] {});
        });

        auto btn_paste1 = std::make_shared<ActionBtn>("📥 Paste", [this]() {
            code_controller_->pasteFromClipboard();
            setState([this] {});
        });

        auto btn_sel_all1 = std::make_shared<ActionBtn>("🔍 Select All", [this]() {
            code_controller_->selectAll();
            setState([this] {});
        });

        auto btn_undo1 = std::make_shared<ActionBtn>("↩ Undo", [this]() {
            code_controller_->undo();
            setState([this] {});
        });

        auto btn_redo1 = std::make_shared<ActionBtn>("↪ Redo", [this]() {
            code_controller_->redo();
            setState([this] {});
        });

        auto toolbar1 = row({
            .gap = StyleValue::point(6.0f),
            .children = {btn_copy1, btn_cut1, btn_paste1, btn_sel_all1, btn_undo1, btn_redo1}
        });

        auto code_editor = TextArea {
            .controller = code_controller_,
            .style = TextStyle{
                .color = 0xFFF1F5F9,
                .font_size = 13.5f,
                .font_family = "monospace"
            },
            .min_lines = 11,
            .max_lines = 16,
            .show_line_numbers = true,
            .show_counter = true,
            .background_color = 0xFF0F172A,
            .border_color = 0xFF334155,
            .focus_border_color = 0xFF38BDF8,
            .on_cursor_moved = [this](size_t r, size_t c) {
                cursor_info_ = "Ln " + std::to_string(r) + ", Col " + std::to_string(c);
                setState([this] {});
            }
        };

        auto code_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {code_header, toolbar1, code_editor}
            })
        });

        // ── 2. Notes Editor Section (Word Counter) ───────────────────
        auto notes_header = text("2. Notes & Documents (Word & Char Counter)", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        // Notes Editor Toolbar
        auto btn_copy2 = std::make_shared<ActionBtn>("📋 Copy", [this]() {
            notes_controller_->copyToClipboard();
            setState([this] {});
        });

        auto btn_cut2 = std::make_shared<ActionBtn>("✂️ Cut", [this]() {
            notes_controller_->cutToClipboard();
            setState([this] {});
        });

        auto btn_paste2 = std::make_shared<ActionBtn>("📥 Paste", [this]() {
            notes_controller_->pasteFromClipboard();
            setState([this] {});
        });

        auto btn_sel_all2 = std::make_shared<ActionBtn>("🔍 Select All", [this]() {
            notes_controller_->selectAll();
            setState([this] {});
        });

        auto btn_undo2 = std::make_shared<ActionBtn>("↩ Undo", [this]() {
            notes_controller_->undo();
            setState([this] {});
        });

        auto btn_redo2 = std::make_shared<ActionBtn>("↪ Redo", [this]() {
            notes_controller_->redo();
            setState([this] {});
        });

        auto toolbar2 = row({
            .gap = StyleValue::point(6.0f),
            .children = {btn_copy2, btn_cut2, btn_paste2, btn_sel_all2, btn_undo2, btn_redo2}
        });

        auto notes_editor = TextArea {
            .controller = notes_controller_,
            .style = TextStyle{
                .color = 0xFFF1F5F9,
                .font_size = 13.5f,
            },
            .min_lines = 11,
            .max_lines = 16,
            .show_line_numbers = false,
            .show_counter = true,
            .max_characters = 600,
            .background_color = 0xFF0F172A,
            .border_color = 0xFF334155,
            .focus_border_color = 0xFF10B981,
        };

        auto notes_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {notes_header, toolbar2, notes_editor}
            })
        });

        // Grid / Row of two editors
        auto editors_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(18.0f),
            .children = {code_box, notes_box}
        });

        // Main Stack Column
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {title_col, editors_row}
        });

        return container({
            .color = 0xFF0B1120,
            .padding = StyleInsets::all(20.0f),
            .flex_grow = 1.0f,
            .child = main_col
        });
    }
};

class TextAreaDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TextAreaDemoState>();
    }
    std::string_view typeName() const override { return "TextAreaDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced TextArea Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced TextArea Demo";
    config.width       = 1160;
    config.height      = 630;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<TextAreaDemoApp>(), config);
}
