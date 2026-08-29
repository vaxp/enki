/// @file test_sliver.cpp
/// @brief Unit & Integration Tests for Section 14 Scrolling — Extended (Slivers).

#include "enki/widgets/sliver.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/card.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Test 1: CustomScrollView Declarative Construction & Mounting
// ════════════════════════════════════════════════════════════════
void test_custom_scroll_view_construction() {
    std::cout << "Testing CustomScrollView Construction & Mounting..." << std::endl;

    WidgetPtr csv = CustomScrollView {
        .slivers = {
            SliverToBoxAdapter {
                .child = container({
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(80.0f),
                    .child = text("Top Banner"),
                }),
            },
            SliverList {
                .item_count = 5,
                .item_builder = [](int idx) {
                    return container({
                        .height = StyleValue::point(40.0f),
                        .child = text("Item " + std::to_string(idx)),
                    });
                },
            },
        },
        .direction = Axis::Vertical,
        .scroll_physics = ScrollPhysics::Clamped,
        .scroll_speed = 60.0f,
        .show_scrollbar = true,
    };

    assert(csv != nullptr);
    assert(csv->typeName() == "CustomScrollView");

    auto el = csv->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();

    // Verify element tree builds properly
    assert(el->findRenderObject() != nullptr);

    el->unmount();
    std::cout << "  ✓ CustomScrollView Construction passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 2: SliverList Static Items and Builder Mode with Separators
// ════════════════════════════════════════════════════════════════
void test_sliver_list() {
    std::cout << "Testing SliverList Widget..." << std::endl;

    // Static mode
    WidgetPtr sl_static = SliverList {
        .items = {
            text("Row 1"),
            text("Row 2"),
            text("Row 3"),
        },
        .padding = EdgeInsets::all(8.0f),
    };
    assert(sl_static != nullptr);
    assert(sl_static->typeName() == "SliverList");

    auto el_static = sl_static->createElement();
    el_static->mount(nullptr, 0);
    el_static->rebuild();
    assert(el_static->findRenderObject() != nullptr);
    el_static->unmount();

    // Builder mode with separator
    int built_count = 0;
    int sep_count = 0;
    WidgetPtr sl_builder = SliverList {
        .item_count = 10,
        .item_builder = [&](int idx) {
            built_count++;
            return text("Item " + std::to_string(idx));
        },
        .separator_builder = [&](int idx) {
            sep_count++;
            return container({.color = 0xFF444444, .height = StyleValue::point(1.0f)});
        },
    };

    auto el_builder = sl_builder->createElement();
    el_builder->mount(nullptr, 0);
    el_builder->rebuild();
    assert(built_count == 10);
    assert(sep_count == 9); // 9 separators between 10 items
    assert(el_builder->findRenderObject() != nullptr);
    el_builder->unmount();

    std::cout << "  ✓ SliverList passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 3: SliverGrid Fixed Count & Max Extent Delegates
// ════════════════════════════════════════════════════════════════
void test_sliver_grid() {
    std::cout << "Testing SliverGrid Widget..." << std::endl;

    // Fixed count delegate
    WidgetPtr grid_fixed = SliverGrid {
        .item_count = 6,
        .item_builder = [](int idx) {
            return container({
                .color = 0xFF2563EB,
                .child = text("Cell " + std::to_string(idx)),
            });
        },
        .fixed_delegate = SliverGridDelegateFixedCount(3, 6.0f, 6.0f, 1.2f),
        .padding = EdgeInsets::all(12.0f),
    };

    assert(grid_fixed != nullptr);
    assert(grid_fixed->typeName() == "SliverGrid");

    auto el_fixed = grid_fixed->createElement();
    el_fixed->mount(nullptr, 0);
    el_fixed->rebuild();
    assert(el_fixed->findRenderObject() != nullptr);
    el_fixed->unmount();

    // Max extent delegate
    WidgetPtr grid_max = SliverGrid {
        .item_count = 8,
        .item_builder = [](int idx) {
            return Card {
                .child = text("Card " + std::to_string(idx)),
            };
        },
        .max_delegate = SliverGridDelegateMaxExtent(180.0f, 8.0f, 8.0f, 1.0f),
        .use_max_extent_delegate = true,
    };

    auto el_max = grid_max->createElement();
    el_max->mount(nullptr, 0);
    el_max->rebuild();
    assert(el_max->findRenderObject() != nullptr);
    el_max->unmount();

    std::cout << "  ✓ SliverGrid passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 4: SliverAppBar Collapsible & Pinned Header
// ════════════════════════════════════════════════════════════════
void test_sliver_app_bar() {
    std::cout << "Testing SliverAppBar Widget..." << std::endl;

    WidgetPtr app_bar = SliverAppBar {
        .title = text("Enki Gallery"),
        .leading = text("<"),
        .actions = {
            text("Search"),
            text("More"),
        },
        .flexible_space = container({
            .color = 0xFF1E293B,
            .child = text("Flexible Header Background"),
        }),
        .expanded_height = 220.0f,
        .collapsed_height = 60.0f,
        .pinned = true,
        .floating = false,
        .center_title = true,
        .background_color = 0xFF0F172A,
        .elevation = 4.0f,
    };

    assert(app_bar != nullptr);
    assert(app_bar->typeName() == "SliverAppBar");

    auto el = app_bar->createElement();
    assert(el != nullptr);
    el->mount(nullptr, 0);
    el->rebuild();
    assert(el->findRenderObject() != nullptr);
    el->unmount();

    std::cout << "  ✓ SliverAppBar passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 5: SliverToBoxAdapter & SliverPadding
// ════════════════════════════════════════════════════════════════
void test_sliver_adapters() {
    std::cout << "Testing SliverToBoxAdapter & SliverPadding..." << std::endl;

    WidgetPtr box = container({
        .color = 0xFF334155,
        .width = StyleValue::percent(100.0f),
        .height = StyleValue::point(100.0f),
        .child = text("Box Adapter Content"),
    });

    WidgetPtr adapter = SliverToBoxAdapter {
        .child = box,
    };
    assert(adapter != nullptr);
    assert(adapter->typeName() == "SliverToBoxAdapter");

    auto el_adapter = adapter->createElement();
    el_adapter->mount(nullptr, 0);
    el_adapter->rebuild();
    assert(el_adapter->findRenderObject() != nullptr);
    el_adapter->unmount();

    WidgetPtr padded_sliver = SliverPadding {
        .padding = EdgeInsets::symmetric(16.0f, 24.0f),
        .sliver = adapter,
    };
    assert(padded_sliver != nullptr);
    assert(padded_sliver->typeName() == "SliverPadding");

    auto el_pad = padded_sliver->createElement();
    el_pad->mount(nullptr, 0);
    el_pad->rebuild();
    assert(el_pad->findRenderObject() != nullptr);
    el_pad->unmount();

    std::cout << "  ✓ SliverToBoxAdapter & SliverPadding passed." << std::endl;
}

// ════════════════════════════════════════════════════════════════
// Test 6: Full Integration — CustomScrollView with Multi-Slivers
// ════════════════════════════════════════════════════════════════
void test_full_sliver_integration() {
    std::cout << "Testing Full Sliver Integration..." << std::endl;

    WidgetPtr full_view = CustomScrollView {
        .slivers = {
            SliverAppBar {
                .title = text("Header Title"),
                .expanded_height = 200.0f,
                .collapsed_height = 56.0f,
                .pinned = true,
            },
            SliverPadding {
                .padding = EdgeInsets::all(12.0f),
                .sliver = SliverGrid {
                    .item_count = 6,
                    .item_builder = [](int i) { return text("Grid " + std::to_string(i)); },
                    .fixed_delegate = SliverGridDelegateFixedCount(2, 8.0f, 8.0f, 1.0f),
                },
            },
            SliverToBoxAdapter {
                .child = container({
                    .padding = StyleInsets::all(16.0f),
                    .child = text("Section Divider / Header"),
                }),
            },
            SliverList {
                .item_count = 20,
                .item_builder = [](int i) { return text("List Row " + std::to_string(i)); },
            },
        },
        .direction = Axis::Vertical,
        .scroll_physics = ScrollPhysics::Clamped,
        .show_scrollbar = true,
    };

    assert(full_view != nullptr);

    auto el = full_view->createElement();
    el->mount(nullptr, 0);
    el->rebuild();
    assert(el->findRenderObject() != nullptr);
    el->unmount();

    std::cout << "  ✓ Full Sliver Integration passed." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     RUNNING EXTENDED SLIVER TESTS      " << std::endl;
    std::cout << "========================================" << std::endl;

    test_custom_scroll_view_construction();
    test_sliver_list();
    test_sliver_grid();
    test_sliver_app_bar();
    test_sliver_adapters();
    test_full_sliver_integration();

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL 6 SLIVER TESTS PASSED!        " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
