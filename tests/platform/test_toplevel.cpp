/// @file test_toplevel.cpp
/// @brief Unit tests for Foreign Toplevel Management abstractions and WindowState.

#include "enki/platform/toplevel.hpp"
#include "enki/core/signal.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

void test_window_state_bitmask() {
    using namespace enki;

    WindowState state = WindowState::Normal;
    assert(state == WindowState::Normal);
    assert(!hasWindowState(state, WindowState::Maximized));
    assert(!hasWindowState(state, WindowState::Minimized));
    assert(!hasWindowState(state, WindowState::Activated));
    assert(!hasWindowState(state, WindowState::Fullscreen));

    // Test OR operator
    state |= WindowState::Maximized;
    assert(hasWindowState(state, WindowState::Maximized));
    assert(!hasWindowState(state, WindowState::Activated));

    state |= WindowState::Activated;
    assert(hasWindowState(state, WindowState::Maximized));
    assert(hasWindowState(state, WindowState::Activated));

    // Test AND operator
    WindowState mask = state & WindowState::Maximized;
    assert(mask == WindowState::Maximized);

    // Test unsetting flag
    state = static_cast<WindowState>(static_cast<uint32_t>(state) & ~static_cast<uint32_t>(WindowState::Maximized));
    assert(!hasWindowState(state, WindowState::Maximized));
    assert(hasWindowState(state, WindowState::Activated));

    std::cout << "[PASS] test_window_state_bitmask\n";
}

void test_memory_toplevel_actions() {
    using namespace enki;

    auto win = std::make_shared<MemoryToplevelWindow>(101, "Firefox", "org.mozilla.firefox", WindowState::Normal);
    assert(win->id() == 101);
    assert(win->title() == "Firefox");
    assert(win->appId() == "org.mozilla.firefox");
    assert(win->state() == WindowState::Normal);
    assert(!win->isActivated());
    assert(!win->isMaximized());
    assert(!win->isMinimized());
    assert(!win->isFullscreen());

    // Activate
    win->activate();
    assert(win->isActivated());

    // Maximize
    win->setMaximized(true);
    assert(win->isMaximized());
    assert(win->isActivated());

    win->setMaximized(false);
    assert(!win->isMaximized());

    // Minimize (should drop active focus in memory mock)
    win->setMinimized(true);
    assert(win->isMinimized());
    assert(!win->isActivated());

    win->setMinimized(false);
    assert(!win->isMinimized());

    // Fullscreen
    win->setFullscreen(true);
    assert(win->isFullscreen());
    win->setFullscreen(false);
    assert(!win->isFullscreen());

    // Title and AppId changes
    win->setTitle("GitHub - Pull Requests");
    assert(win->title() == "GitHub - Pull Requests");

    win->setAppId("firefox-developer");
    assert(win->appId() == "firefox-developer");

    // Close
    assert(!win->isClosed());
    win->close();
    assert(win->isClosed());

    std::cout << "[PASS] test_memory_toplevel_actions\n";
}

void test_toplevel_signals() {
    using namespace enki;

    Signal<std::shared_ptr<ToplevelWindow>> sig_created;
    Signal<std::shared_ptr<ToplevelWindow>> sig_closed;
    Signal<std::shared_ptr<ToplevelWindow>> sig_active;
    Signal<std::shared_ptr<ToplevelWindow>, std::string_view> sig_title;
    Signal<std::shared_ptr<ToplevelWindow>, WindowState> sig_state;

    int created_count = 0;
    int closed_count = 0;
    int active_count = 0;
    std::string latest_title;
    WindowState latest_state = WindowState::Normal;

    sig_created.connect([&](std::shared_ptr<ToplevelWindow> /*w*/) { ++created_count; });
    sig_closed.connect([&](std::shared_ptr<ToplevelWindow> /*w*/) { ++closed_count; });
    sig_active.connect([&](std::shared_ptr<ToplevelWindow> /*w*/) { ++active_count; });
    sig_title.connect([&](std::shared_ptr<ToplevelWindow> /*w*/, std::string_view t) { latest_title = t; });
    sig_state.connect([&](std::shared_ptr<ToplevelWindow> /*w*/, WindowState s) { latest_state = s; });

    auto win = std::make_shared<MemoryToplevelWindow>(202, "Terminal", "kitty");
    sig_created.emit(win);
    assert(created_count == 1);

    sig_title.emit(win, "zsh ~");
    assert(latest_title == "zsh ~");

    sig_state.emit(win, WindowState::Activated | WindowState::Maximized);
    assert(hasWindowState(latest_state, WindowState::Activated));
    assert(hasWindowState(latest_state, WindowState::Maximized));

    sig_active.emit(win);
    assert(active_count == 1);

    sig_closed.emit(win);
    assert(closed_count == 1);

    std::cout << "[PASS] test_toplevel_signals\n";
}

int main() {
    test_window_state_bitmask();
    test_memory_toplevel_actions();
    test_toplevel_signals();
    std::cout << "All Foreign Toplevel unit tests passed successfully!\n";
    return 0;
}
