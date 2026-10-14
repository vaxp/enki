#include "enki/widgets/flyout.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include <cassert>
#include <iostream>

int main() {
    std::cout << "[Test Flyout] Running Shell Flyout subsystem tests...\n";

    // 1. Test FlyoutOptions defaults
    enki::FlyoutOptions options;
    assert(options.width == 360);
    assert(options.height == 480);
    assert(options.auto_dismiss == true);
    assert(options.placement == enki::FlyoutPlacement::Auto);
    assert(options.layer == enki::ShellLayer::Top);

    enki::FlyoutOptions cursor_options;
    cursor_options.placement = enki::FlyoutPlacement::AtCursor;
    cursor_options.custom_position = enki::Point{250.0f, 350.0f};
    assert(cursor_options.placement == enki::FlyoutPlacement::AtCursor);
    assert(cursor_options.custom_position.has_value());
    assert(cursor_options.custom_position->x == 250.0f);
    std::cout << "  ✓ FlyoutOptions defaults & AtCursor verified\n";

    // 2. Test FlyoutController state transitions
    auto controller = std::make_shared<enki::FlyoutController>();
    assert(!controller->isOpen());

    bool callback_called = false;
    bool last_state = false;
    controller->setToggleCallback([&](bool is_open) {
        callback_called = true;
        last_state = is_open;
    });

    controller->show();
    assert(controller->isOpen());
    assert(callback_called && last_state == true);

    controller->hide();
    assert(!controller->isOpen());
    assert(last_state == false);

    controller->toggle();
    assert(controller->isOpen());
    assert(last_state == true);
    std::cout << "  ✓ FlyoutController state management verified\n";

    // 3. Test WindowConfig with Flyout mode
    enki::WindowConfig cfg;
    cfg.mode = enki::WindowMode::Flyout;
    cfg.width = options.width;
    cfg.height = options.height;
    cfg.exclusive_zone = -1;
    cfg.layer = enki::ShellLayer::Top;
    cfg.anchor = enki::ShellAnchor::TopRight;
    cfg.margin.top = 40;
    cfg.margin.right = 16;

    assert(cfg.mode == enki::WindowMode::Flyout);
    assert(cfg.exclusive_zone == -1);
    assert(cfg.anchor == enki::ShellAnchor::TopRight);
    assert(cfg.margin.top == 40 && cfg.margin.right == 16);
    std::cout << "  ✓ WindowConfig Flyout parameters verified\n";

    // 4. Test Declarative Flyout DSL Widget
    auto flyout_widget = enki::Flyout{
        .child = enki::text("Control Center Button"),
        .builder = [](enki::BuildContext&, std::shared_ptr<enki::FlyoutHandle>) {
            return enki::container({
                .child = enki::text("Control Center Drawer"),
            });
        },
        .options = enki::FlyoutOptions{
            .width = 380,
            .height = 540,
            .auto_dismiss = true,
            .border_shader = "vec4 main(vec2 c) { return vec4(1.0); }",
            .box_shadow = { enki::BoxShadow::glow(0x608B5CF6, 20.0f) },
        },
        .controller = controller,
    };

    assert(flyout_widget.options.border_shader == "vec4 main(vec2 c) { return vec4(1.0); }");
    assert(!flyout_widget.options.box_shadow.empty());

    enki::WidgetPtr ptr = flyout_widget;
    assert(ptr != nullptr);
    assert(ptr->typeName() == "FlyoutWidget");
    std::cout << "  ✓ Declarative Flyout DSL & Decoration verified\n";

    std::cout << "[Test Flyout] All tests passed successfully!\n";
    return 0;
}
