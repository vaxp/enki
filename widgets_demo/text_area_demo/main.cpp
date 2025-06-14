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

        auto txt = text(btn->label);
        txt->fontSize(11.5f).bold().color(0xFFFFFFFF);

        auto box = container(txt);
        box->color(hovered_ ? 0xFF475569 : btn->bg_color)
           .borderRadius(6.0f)
           .paddingSymmetric(5.0f, 10.0f);

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Pointer;

        detector->on_hover_enter = [this](const PointerEvent&) { setState([this] { hovered_ = true; }); };
        detector->on_hover_exit  = [this](const PointerEvent&) { setState([this] { hovered_ = false; }); };
        detector->on_tap = [btn] { if (btn->on_press) btn->on_press(); };

        detector->child = box;
        return detector;
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
        auto title = text("Advanced TextArea Widget Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Multi-line text editor with line numbers, clipboard (Copy/Cut/Paste), undo/redo, and word count");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── 1. Code Editor Section (With Line Numbers) ─────────────────
        auto code_header = text("1. Code / Script Editor (Line Numbers & Monospace)");
        code_header->fontSize(14.0f).bold().color(0xFF38BDF8);

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

        std::vector<WidgetPtr> tb1_items = {btn_copy1, btn_cut1, btn_paste1, btn_sel_all1, btn_undo1, btn_redo1};
        auto toolbar1 = row(tb1_items);
        toolbar1->gap(StyleValue::point(6.0f));

        TextAreaOptions code_opts;
        code_opts.style.font_family = "monospace";
        code_opts.style.font_size   = 13.5f;
        code_opts.style.color       = 0xFFF1F5F9;
        code_opts.show_line_numbers = true;
        code_opts.show_counter      = true;
        code_opts.min_lines         = 11;
        code_opts.max_lines         = 16;
        code_opts.background_color  = 0xFF0F172A;
        code_opts.border_color      = 0xFF334155;
        code_opts.focus_border_color = 0xFF38BDF8;
        code_opts.on_cursor_moved = [this](size_t row, size_t col) {
            cursor_info_ = "Ln " + std::to_string(row) + ", Col " + std::to_string(col);
            setState([this] {});
        };

        auto code_editor = textArea(code_controller_, code_opts);

        std::vector<WidgetPtr> code_section_items = {code_header, toolbar1, code_editor};
        auto code_col = column(code_section_items);
        code_col->gap(StyleValue::point(10.0f));

        auto code_box = container(code_col);
        code_box->color(0xFF1E293B)
                .borderRadius(10.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(16.0f)
                .width(530.0f);

        // ── 2. Notes Editor Section (Word Counter) ───────────────────
        auto notes_header = text("2. Notes & Documents (Word & Char Counter)");
        notes_header->fontSize(14.0f).bold().color(0xFF10B981);

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

        std::vector<WidgetPtr> tb2_items = {btn_copy2, btn_cut2, btn_paste2, btn_sel_all2, btn_undo2, btn_redo2};
        auto toolbar2 = row(tb2_items);
        toolbar2->gap(StyleValue::point(6.0f));

        TextAreaOptions notes_opts;
        notes_opts.style.font_size   = 13.5f;
        notes_opts.style.color       = 0xFFF1F5F9;
        notes_opts.show_line_numbers = false;
        notes_opts.show_counter      = true;
        notes_opts.max_characters    = 600;
        notes_opts.min_lines         = 11;
        notes_opts.max_lines         = 16;
        notes_opts.background_color  = 0xFF0F172A;
        notes_opts.border_color      = 0xFF334155;
        notes_opts.focus_border_color = 0xFF10B981;

        auto notes_editor = textArea(notes_controller_, notes_opts);

        std::vector<WidgetPtr> notes_section_items = {notes_header, toolbar2, notes_editor};
        auto notes_col = column(notes_section_items);
        notes_col->gap(StyleValue::point(10.0f));

        auto notes_box = container(notes_col);
        notes_box->color(0xFF1E293B)
                 .borderRadius(10.0f)
                 .border(0xFF334155, 1.0f)
                 .paddingAll(16.0f)
                 .width(530.0f);

        // Grid / Row of two editors
        std::vector<WidgetPtr> editors_row_items = {code_box, notes_box};
        auto editors_row = row(editors_row_items);
        editors_row->gap(StyleValue::point(18.0f))
                   .justifyContent(Justify::Center);

        // Main Stack Column
        std::vector<WidgetPtr> main_col_items = {title_col, editors_row};
        auto main_col = column(main_col_items);
        main_col->gap(StyleValue::point(18.0f))
                .alignItems(Align::Center);

        auto app_root = container(main_col);
        app_root->color(0xFF0B1120)
                .paddingAll(20.0f)
                .flexGrow(1.0f);

        return app_root;
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
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<TextAreaDemoApp>(), config);
}
