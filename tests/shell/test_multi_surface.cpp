/// @file test_multi_surface.cpp
/// @brief Tests for ShellApp, SurfaceHost, and NativePopup.

#include "enki/shell/shell_app.hpp"
#include "enki/shell/surface_host.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <cassert>
#include <iostream>

using namespace enki;

void test_shell_app_creation() {
    std::cout << "[TEST] Running test_shell_app_creation...\n";
    auto app_res = ShellApp::create();
    assert(app_res.isOk() && "ShellApp creation must succeed");
    auto app = std::move(app_res.value());
    assert(ShellApp::instance() == app.get() && "Singleton instance must match");
    assert(app->surfaceCount() == 0 && "Initial surface count must be 0");
    std::cout << "  ✓ ShellApp creation passed\n";
}

void test_multi_surface_hosts() {
    std::cout << "[TEST] Running test_multi_surface_hosts...\n";
    auto app_res = ShellApp::create();
    assert(app_res.isOk());
    auto app = std::move(app_res.value());

    // Create Surface 1: Top Bar (36px high)
    auto bar_wg = container(std::make_shared<Text>("Bar Title"));
    bar_wg->width(800.0f).height(36.0f);

    WindowConfig win_cfg1;
    win_cfg1.title  = "Bar Surface";
    win_cfg1.width  = 800;
    win_cfg1.height = 36;
    SurfaceHost* host1 = app->addWindow(win_cfg1, bar_wg);
    assert(host1 != nullptr && "Host 1 must be created");
    assert(app->surfaceCount() == 1);

    // Create Surface 2: Dock (64px high)
    auto dock_wg = container(std::make_shared<Text>("Dock Apps"));
    dock_wg->width(400.0f).height(64.0f);

    WindowConfig win_cfg2;
    win_cfg2.title  = "Dock Surface";
    win_cfg2.width  = 400;
    win_cfg2.height = 64;
    SurfaceHost* host2 = app->addWindow(win_cfg2, dock_wg);
    assert(host2 != nullptr && "Host 2 must be created");
    assert(app->surfaceCount() == 2);

    // Test layout on both surfaces independently
    host1->rebuild();
    host1->layout();
    assert(host1->getRootElement() != nullptr);
    assert(host1->getRootElement()->findRenderObject() != nullptr);

    host2->rebuild();
    host2->layout();
    assert(host2->getRootElement() != nullptr);
    assert(host2->getRootElement()->findRenderObject() != nullptr);

    // Remove one surface
    app->removeSurface(host1);
    assert(app->surfaceCount() == 1);

    std::cout << "  ✓ Multi-surface hosts passed\n";
}

void test_native_popup_spawn() {
    std::cout << "[TEST] Running test_native_popup_spawn...\n";
    auto app_res = ShellApp::create();
    assert(app_res.isOk());
    auto app = std::move(app_res.value());

    // Create Bar
    WindowConfig bar_cfg;
    bar_cfg.width = 1920;
    bar_cfg.height = 36;
    SurfaceHost* bar_host = app->addWindow(bar_cfg, container());
    assert(bar_host != nullptr);

    // Spawn native popup outside bar
    PopupOptions opts;
    opts.anchor_rect = Rect{100.0f, 0.0f, 80.0f, 36.0f}; // Button at (100, 0, 80, 36)
    opts.placement   = PopupPlacement::BottomStart;
    opts.width       = 240;
    opts.height      = 300;
    opts.offset_gap  = 6;

    bool closed = false;
    opts.on_close = [&closed] { closed = true; };

    // Build context dummy
    auto dummy_wg = container();
    auto dummy_el = dummy_wg->createElement();
    BuildContext ctx(dummy_el.get());

    auto popup = NativePopup::show(
        ctx,
        opts,
        [](BuildContext&) -> WidgetPtr {
            return container(std::make_shared<Text>("Popup Menu Content"));
        }
    );

    assert(popup != nullptr && "Popup must be created");
    assert(popup->isOpen() && "Popup must be open");
    assert(app->surfaceCount() == 2 && "App must now manage 2 surfaces (Bar + Popup)");

    // Close popup
    popup->close();
    assert(!popup->isOpen() && "Popup must now be closed");
    assert(closed && "on_close callback must have executed");
    assert(app->surfaceCount() == 1 && "App surface count must return to 1");

    std::cout << "  ✓ Native popup spawn passed\n";
}

void test_bar_button_click_and_popup() {
    std::cout << "[TEST] Running test_bar_button_click_and_popup...\n";
    auto app_res = ShellApp::create();
    assert(app_res.isOk());
    auto app = std::move(app_res.value());

    bool clicked = false;
    auto detector = std::make_shared<GestureDetector>();
    detector->hit_test_behavior = HitTestBehavior::Opaque;
    detector->on_tap = [&clicked] {
        clicked = true;
    };
    auto btn_box = container(std::make_shared<Text>("ClickMe"));
    btn_box->width(100.0f).height(36.0f);
    detector->child = btn_box;

    auto bar = container(detector);
    bar->width(1200.0f).height(36.0f);

    WindowConfig bar_win;
    bar_win.width = 1200;
    bar_win.height = 36;
    SurfaceHost* host = app->addWindow(bar_win, bar);
    assert(host != nullptr);

    // Initial frame
    host->rebuild();
    host->layout();

    // Now simulate mouse down and up on the button at (20, 18)
    host->handlePointerDown(20.0f, 18.0f, MouseButton::Left);
    host->handlePointerUp(20.0f, 18.0f, MouseButton::Left);

    std::cout << "Clicked status: " << (clicked ? "TRUE" : "FALSE") << "\n";
    assert(clicked && "Bar button click must fire on_tap callback");
    std::cout << "  ✓ Bar button click passed\n";
}

void dumpRenderTree(RenderObject* ro, int depth = 0) {
    if (!ro) return;
    std::string indent(depth * 2, ' ');
    Rect gb = ro->globalBounds();
    std::cout << indent << "- " << ro->debugDescription()
              << " | globalBounds: (" << gb.x << ", " << gb.y << ", " << gb.width << ", " << gb.height << ")\n";
    for (auto* child : ro->children()) {
        dumpRenderTree(child, depth + 1);
    }
}

// Replicate BarButton from demo
class TestBarButton : public StatefulWidget {
public:
    std::string icon;
    std::string label;
    std::function<void(BuildContext&, Rect)> on_press;

    TestBarButton(std::string icon, std::string label, std::function<void(BuildContext&, Rect)> on_press)
        : icon(std::move(icon)), label(std::move(label)), on_press(std::move(on_press)) {}

    std::string_view typeName() const override { return "TestBarButton"; }
    std::unique_ptr<State> createState() override;
};

class TestBarButtonState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const TestBarButton*>(widget());

        std::vector<WidgetPtr> items;
        if (!w->icon.empty()) {
            items.push_back(std::make_shared<Text>(w->icon, TextStyle{.font_size = 13.0f}));
        }
        if (!w->label.empty()) {
            items.push_back(std::make_shared<Text>(w->label, TextStyle{.font_size = 12.0f}));
        }

        auto content_row = row(std::move(items));
        content_row->gap(StyleValue::point(6.0f));

        auto box = container(content_row);
        box->paddingSymmetric(4.0f, 10.0f);

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Pointer;

        auto on_click = w->on_press;
        detector->on_tap = [this, on_click] {
            if (on_click) {
                Rect btn_rect{12.0f, 0.0f, 110.0f, 36.0f};
                on_click(context(), btn_rect);
            }
        };

        detector->child = box;
        return detector;
    }
};

std::unique_ptr<State> TestBarButton::createState() {
    return std::make_unique<TestBarButtonState>();
}

void test_full_desktop_shell_tree() {
    std::cout << "[TEST] Running test_full_desktop_shell_tree...\n";
    auto app_res = ShellApp::create();
    assert(app_res.isOk());
    auto app = std::move(app_res.value());

    bool launcher_clicked = false;
    bool wifi_clicked = false;

    auto launcher_btn = std::make_shared<TestBarButton>(
        "⚡", "Applications",
        [&](BuildContext&, Rect) {
            launcher_clicked = true;
        }
    );
    auto ws1 = std::make_shared<TestBarButton>("1", "", nullptr);
    auto ws2 = std::make_shared<TestBarButton>("2", "", nullptr);
    auto ws3 = std::make_shared<TestBarButton>("3", "", nullptr);

    auto left_row = row({launcher_btn, ws1, ws2, ws3});
    left_row->gap(StyleValue::point(4.0f));

    auto time_t = std::make_shared<Text>("Sat Aug 8  22:30", TextStyle{.font_size = 13.0f});
    auto center_box = container(time_t);
    center_box->paddingSymmetric(4.0f, 12.0f);

    auto wifi_btn = std::make_shared<TestBarButton>(
        "📶", "Connected",
        [&](BuildContext&, Rect) {
            wifi_clicked = true;
        }
    );
    auto vol_btn = std::make_shared<TestBarButton>("🔊", "75%", nullptr);
    auto bat_btn = std::make_shared<TestBarButton>("🔋", "92%", nullptr);

    auto right_row = row({wifi_btn, vol_btn, bat_btn});
    right_row->gap(StyleValue::point(6.0f));

    auto bar_row = row({left_row, center_box, right_row});
    bar_row->justifyContent(Justify::SpaceBetween)
            .alignItems(Align::Center)
            .padding(StyleInsets::symmetric(0.0f, 12.0f));

    auto bar_container = container(bar_row);
    bar_container->width(StyleValue::percent(100.0f))
                 .height(36.0f);

    WindowConfig bar_win;
    bar_win.width = 1200;
    bar_win.height = 36;
    SurfaceHost* host = app->addWindow(bar_win, bar_container);
    assert(host != nullptr);

    host->rebuild();
    host->layout();

    std::cout << "--- Dump Render Tree ---\n";
    dumpRenderTree(host->getRootElement()->findRenderObject());
    std::cout << "------------------------\n";

    // Test clicking on launcher button (around x=20, y=18)
    host->handlePointerDown(20.0f, 18.0f, MouseButton::Left);
    host->handlePointerUp(20.0f, 18.0f, MouseButton::Left);

    std::cout << "Launcher clicked: " << (launcher_clicked ? "YES" : "NO") << "\n";
    assert(launcher_clicked && "Launcher button must be clicked");

    std::cout << "  ✓ Full desktop shell tree click test passed\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  ENKI Desktop Shell Tests\n";
    std::cout << "========================================\n";

    test_shell_app_creation();
    test_multi_surface_hosts();
    test_native_popup_spawn();
    test_bar_button_click_and_popup();
    test_full_desktop_shell_tree();

    std::cout << "All Desktop Shell multi-surface tests passed!\n";
    return 0;
}

