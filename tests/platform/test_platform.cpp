/// @file test_platform.cpp
/// @brief Unit tests for Platform configuration and LayerSurface types.

#include "enki/platform/platform.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/platform/window.hpp"
#include <cassert>
#include <iostream>

void test_layer_surface_enums() {
    using namespace enki;

    // Test ShellLayer enum values
    assert(static_cast<uint32_t>(ShellLayer::Background) == 0);
    assert(static_cast<uint32_t>(ShellLayer::Bottom) == 1);
    assert(static_cast<uint32_t>(ShellLayer::Top) == 2);
    assert(static_cast<uint32_t>(ShellLayer::Overlay) == 3);

    // Test ShellAnchor bitwise operators
    ShellAnchor anchor = ShellAnchor::Top | ShellAnchor::Left;
    assert(hasAnchor(anchor, ShellAnchor::Top));
    assert(hasAnchor(anchor, ShellAnchor::Left));
    assert(!hasAnchor(anchor, ShellAnchor::Right));
    assert(!hasAnchor(anchor, ShellAnchor::Bottom));

    ShellAnchor full = ShellAnchor::TopAll;
    assert(hasAnchor(full, ShellAnchor::Top));
    assert(hasAnchor(full, ShellAnchor::Left));
    assert(hasAnchor(full, ShellAnchor::Right));

    // Test KeyboardMode
    assert(static_cast<uint32_t>(KeyboardMode::None) == 0);
    assert(static_cast<uint32_t>(KeyboardMode::Exclusive) == 1);
    assert(static_cast<uint32_t>(KeyboardMode::OnDemand) == 2);

    // Test LayerSurfaceConfig defaults
    LayerSurfaceConfig cfg;
    assert(cfg.namespace_id == "enki-shell");
    assert(cfg.layer == ShellLayer::Top);
    assert(cfg.anchor == ShellAnchor::TopAll);
    assert(cfg.height == 34);
    assert(cfg.keyboard_mode == KeyboardMode::None);
    assert(cfg.transparent == true);

    std::cout << "[PASS] test_layer_surface_enums\n";
}

void test_window_config() {
    using namespace enki;

    WindowConfig cfg;
    assert(cfg.width == 1280);
    assert(cfg.height == 800);
    assert(cfg.resizable == true);
    assert(cfg.vsync == true);

    std::cout << "[PASS] test_window_config\n";
}

int main() {
    test_layer_surface_enums();
    test_window_config();
    std::cout << "All platform unit tests passed!\n";
    return 0;
}
