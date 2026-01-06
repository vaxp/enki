/// @file test_hero.cpp
/// @brief Unit tests for Hero, HeroRegistry, and HeroFlightWidget.

#include "enki/widgets/hero.hpp"
#include "enki/widgets/container.hpp"
#include <cassert>
#include <cstdio>

using namespace enki;

void test_hero_registry_lifecycle() {
    auto& reg = HeroRegistry::instance();
    reg.clear();

    assert(!reg.hasHero("photo_1"));

    RenderHero rh("photo_1");
    auto child = container({ .width = 50.0f, .height = 50.0f });

    reg.registerHero("photo_1", &rh, child);
    assert(reg.hasHero("photo_1"));

    const auto* rec = reg.findHero("photo_1");
    assert(rec != nullptr);
    assert(rec->tag == "photo_1");
    assert(rec->render_object == &rh);

    reg.updateBounds("photo_1", &rh, Rect{10.0f, 20.0f, 100.0f, 100.0f});
    assert(rec->last_global_bounds.x == 10.0f);
    assert(rec->last_global_bounds.width == 100.0f);

    reg.unregisterHero("photo_1", &rh);
    assert(!reg.hasHero("photo_1"));

    printf("  [PASS] hero registry registration & bounds tracking\n");
}

void test_hero_widget_creation() {
    WidgetPtr w = hero({
        .tag = "avatar_42",
        .child = container({
            .color = 0xFF3B82F6,
            .width = 64.0f,
            .height = 64.0f,
        }),
    });

    assert(w != nullptr);
    printf("  [PASS] hero widget declarative creation\n");
}

void test_hero_flight_widget() {
    Rect start{0.0f, 0.0f, 40.0f, 40.0f};
    Rect end{100.0f, 200.0f, 160.0f, 160.0f};

    WidgetPtr shuttle = heroFlight(start, end, 0.5f, container({ .color = 0xFFFF0000 }));
    assert(shuttle != nullptr);

    printf("  [PASS] hero flight widget instantiation\n");
}

int main() {
    printf("Running Hero Transition tests...\n");
    test_hero_registry_lifecycle();
    test_hero_widget_creation();
    test_hero_flight_widget();
    printf("All Hero Transition tests passed successfully!\n");
    return 0;
}
