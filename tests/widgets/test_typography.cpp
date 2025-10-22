/// @file test_typography.cpp
/// @brief Comprehensive Unit & Integration Tests for Section 17 Typography — Extended.

#include "enki/widgets/typography.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/element.hpp"
#include "enki/platform/input.hpp"
#include "enki/platform/platform.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: SelectableText Construction, Mounting & Selection
// ════════════════════════════════════════════════════════════════
void test_selectable_text() {
    std::cout << "Testing SelectableText Widget & Text { .selectable = true }..." << std::endl;

    bool selection_changed = false;
    TextSelection last_selection = TextSelection::empty();

    // 1. Test SelectableText Declarative Wrapper
    WidgetPtr st = SelectableText {
        .text = "Hello Enki Selectable Text Framework",
        .style = TextStyle{
            .color = 0xFFFFFFFF,
            .font_size = 16.0f,
            .font_weight = FontWeight::Medium,
        },
        .selection_color = 0x6038BDF8,
        .on_selection_changed = [&](TextSelection sel) {
            selection_changed = true;
            last_selection = sel;
        },
    };

    assert(st != nullptr);

    auto el = st->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Simulate pointer down and drag selection
    PointerEvent down_event;
    down_event.type = PointerEvent::Down;
    down_event.localPosition = Point{10.0f, 10.0f};
    down_event.button = MouseButton::Left;
    ro->handlePointerDown(down_event);

    PointerEvent move_event;
    move_event.type = PointerEvent::Move;
    move_event.localPosition = Point{80.0f, 10.0f};
    ro->handlePointerMove(move_event);

    assert(selection_changed);
    assert(last_selection.isValid());

    PointerEvent up_event;
    up_event.type = PointerEvent::Up;
    up_event.localPosition = Point{80.0f, 10.0f};
    ro->handlePointerUp(up_event);

    el->unmount();

    // 2. Test Text widget with .selectable = true directly
    bool text_sel_changed = false;
    WidgetPtr direct_text = text(TextProps{
        .text = "Direct selectable text property test",
        .selectable = true,
        .on_selection_changed = [&](TextSelection) {
            text_sel_changed = true;
        },
    });

    auto el2 = direct_text->createElement();
    el2->mount(nullptr, 0);
    el2->rebuild();
    auto* ro2 = el2->findRenderObject();
    assert(ro2 != nullptr);
    ro2->handlePointerDown(down_event);
    ro2->handlePointerMove(move_event);
    assert(text_sel_changed);
    el2->unmount();

    std::cout << "  ✓ SelectableText passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: Marquee Construction, Velocity & Continuous Ticker
// ════════════════════════════════════════════════════════════════
void test_marquee() {
    std::cout << "Testing Marquee Widget..." << std::endl;

    WidgetPtr mq = Marquee {
        .text = "BREAKING NEWS: ENKI Engine Typography Subsystem is fully operational!",
        .style = TextStyle{
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        },
        .velocity = 60.0f,
        .blank_space = 80.0f,
        .direction = MarqueeDirection::RightToLeft,
        .pause_on_hover = true,
        .fading_edge_length = 24.0f,
    };

    assert(mq != nullptr);
    assert(mq->typeName() == "Marquee");

    auto el = mq->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    auto* ro = el->findRenderObject();
    assert(ro != nullptr);

    // Test hover interaction
    PointerEvent enter_event;
    ro->handlePointerEnter(enter_event);

    PointerEvent exit_event;
    ro->handlePointerExit(exit_event);

    el->unmount();
    std::cout << "  ✓ Marquee passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: CodeBlock Syntax Highlighting & Themes
// ════════════════════════════════════════════════════════════════
void test_code_block() {
    std::cout << "Testing CodeBlock Widget..." << std::endl;

    std::string sample_cpp = 
        "#include <iostream>\n"
        "// Main execution entry point\n"
        "int main() {\n"
        "    auto app = customScrollView({\n"
        "        .slivers = { sliverAppBar() }\n"
        "    });\n"
        "    return 0;\n"
        "}";

    WidgetPtr cb_cpp = CodeBlock {
        .code = sample_cpp,
        .language = "cpp",
        .show_line_numbers = true,
        .show_copy_button = true,
        .show_header = true,
        .highlighted_lines = {4, 5},
        .theme = CodeTheme::oneDark(),
    };

    assert(cb_cpp != nullptr);
    assert(cb_cpp->typeName() == "CodeBlock");

    auto el_cpp = cb_cpp->createElement();
    assert(el_cpp != nullptr);
    el_cpp->mount(nullptr, 0);
    el_cpp->rebuild();

    assert(el_cpp->findRenderObject() != nullptr);
    el_cpp->unmount();

    // Test JSON code block
    std::string sample_json = "{\n  \"name\": \"ENKI\",\n  \"version\": 0.2,\n  \"active\": true\n}";
    WidgetPtr cb_json = CodeBlock {
        .code = sample_json,
        .language = "json",
        .theme = CodeTheme::dracula(),
    };

    auto el_json = cb_json->createElement();
    el_json->mount(nullptr, 0);
    el_json->rebuild();
    assert(el_json->findRenderObject() != nullptr);
    el_json->unmount();

    // Test Python code block
    std::string sample_py = "def calculate_speed(distance, time):\n    return distance / time\n";
    WidgetPtr cb_py = CodeBlock {
        .code = sample_py,
        .language = "python",
        .theme = CodeTheme::githubDark(),
    };

    auto el_py = cb_py->createElement();
    el_py->mount(nullptr, 0);
    el_py->rebuild();
    assert(el_py->findRenderObject() != nullptr);
    el_py->unmount();

    std::cout << "  ✓ CodeBlock passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    RUNNING EXTENDED TYPOGRAPHY TESTS   " << std::endl;
    std::cout << "========================================" << std::endl;

    test_selectable_text();
    test_marquee();
    test_code_block();

    std::cout << "========================================" << std::endl;
    std::cout << "    ALL 3 TYPOGRAPHY TESTS PASSED!      " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
