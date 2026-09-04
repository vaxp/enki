/// @file main.cpp
/// @brief ENKI Advanced CommandPalette Fuzzy Search Overlay Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace enki;

class CommandPaletteDemoState : public State {
private:
    std::shared_ptr<CommandPaletteController> palette_ctrl_;
    std::vector<std::string> activity_log_;
    int total_executions_ = 0;
    std::string current_theme_ = "Obsidian Dark";

    void logAction(const std::string& action) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        activity_log_.insert(activity_log_.begin(), "[" + ss.str() + "] " + action);
        if (activity_log_.size() > 8) activity_log_.pop_back();
        total_executions_++;
        setState([] {});
    }

    std::vector<CommandItem> buildSampleCommands() {
        std::vector<CommandItem> items;

        // Navigation
        items.push_back(CommandItem("nav_dash", "Navigation: Go to Main Dashboard", "Navigation", "Ctrl+1", "📊", [this] {
            logAction("Navigated to Main Dashboard");
        }));
        items.push_back(CommandItem("nav_files", "Navigation: Open File Explorer", "Navigation", "Ctrl+Shift+E", "📁", [this] {
            logAction("Opened File Explorer");
        }));
        items.push_back(CommandItem("nav_term", "Navigation: Toggle Integrated Terminal", "Navigation", "Ctrl+`", "💻", [this] {
            logAction("Toggled Integrated Terminal");
        }));
        items.push_back(CommandItem("nav_settings", "Navigation: Open Application Settings", "Navigation", "Ctrl+,", "⚙️", [this] {
            logAction("Opened Application Settings");
        }));

        // Git Operations
        items.push_back(CommandItem("git_pull", "Git: Pull Latest Changes from Origin", "Git Operations", "Ctrl+Shift+P", "📥", [this] {
            logAction("Executed: git pull origin main");
        }));
        items.push_back(CommandItem("git_push", "Git: Push Commits to Remote", "Git Operations", "Ctrl+Shift+U", "📤", [this] {
            logAction("Executed: git push origin main");
        }));
        items.push_back(CommandItem("git_commit", "Git: Commit Staged Changes", "Git Operations", "Ctrl+Enter", "💾", [this] {
            logAction("Executed: git commit -m 'feat: updates'");
        }));
        items.push_back(CommandItem("git_branch", "Git: Create New Feature Branch", "Git Operations", "Ctrl+B", "🌿", [this] {
            logAction("Prompted: Create new Git branch");
        }));

        // Editor Actions
        items.push_back(CommandItem("edit_format", "Editor: Format Document (ClangFormat)", "Editor", "Shift+Alt+F", "✨", [this] {
            logAction("Formatted current buffer with ClangFormat");
        }));
        items.push_back(CommandItem("edit_find", "Editor: Find in Files (Ripgrep)", "Editor", "Ctrl+Shift+F", "🔍", [this] {
            logAction("Launched Ripgrep global search");
        }));
        items.back().keywords = {"grep", "search", "find", "query"};

        items.push_back(CommandItem("edit_dup", "Editor: Duplicate Current Line Down", "Editor", "Shift+Alt+Down", "📋", [this] {
            logAction("Duplicated current line");
        }));

        // Appearance & Themes
        items.push_back(CommandItem("theme_obsidian", "Theme: Switch to Obsidian Dark", "Appearance", "Alt+1", "🌙", [this] {
            current_theme_ = "Obsidian Dark";
            logAction("Changed theme to Obsidian Dark");
        }));
        items.push_back(CommandItem("theme_cyber", "Theme: Switch to Cyber Neon Cyan", "Appearance", "Alt+2", "⚡", [this] {
            current_theme_ = "Cyber Neon Cyan";
            logAction("Changed theme to Cyber Neon Cyan");
        }));
        items.push_back(CommandItem("theme_solar", "Theme: Switch to Solarized Deep", "Appearance", "Alt+3", "☀️", [this] {
            current_theme_ = "Solarized Deep";
            logAction("Changed theme to Solarized Deep");
        }));

        // System & Dangerous Actions
        items.push_back(CommandItem("sys_reload", "System: Hot Reload UI Trees", "System", "Ctrl+R", "🔄", [this] {
            logAction("Triggered hot reload of active UI trees");
        }));
        items.push_back(CommandItem("sys_cache", "System: Clear Local Disk Cache", "System", "Ctrl+Shift+Del", "🧹", [this] {
            logAction("Cleared 128 MB of cached assets");
        }));

        CommandItem danger_cmd("sys_reset", "System: Factory Reset Workspace Data", "System", "", "⚠️", [this] {
            logAction("WARNING: Factory reset requested!");
        });
        danger_cmd.is_danger = true;
        danger_cmd.badge = "Danger";
        items.push_back(danger_cmd);

        return items;
    }

public:
    void initState() override {
        State::initState();
        palette_ctrl_ = std::make_shared<CommandPaletteController>();
        activity_log_.push_back("[System] CommandPalette Engine initialized. Press Ctrl+K anytime.");
    }

    WidgetPtr build(BuildContext&) override {
        // ── 1. Header Navigation Bar ──────────────────────────────────
        auto logo = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                container({
                    .color = 0xFF0284C7,
                    .border_radius = BorderRadius::circular(8.0f),
                    .padding = StyleInsets::all(8.0f),
                    .child = text("⌘", { .color = 0xFFFFFFFF, .font_size = 18.0f, .font_weight = FontWeight::Bold })
                }),
                column({
                    .gap = StyleValue::point(2.0f),
                    .children = {
                        text("ENKI Studio", { .color = 0xFFF8FAFC, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                        text("Roadmap v0.2.0 • Section 19 Overlay Extended", { .color = 0xFF64748B, .font_size = 11.0f })
                    }
                })
            }
        });

        auto search_button = gestureDetector({
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(14.0f),
                    .children = {
                        row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text("🔍", { .font_size = 13.0f }),
                                text("Search commands or actions...", { .color = 0xFF94A3B8, .font_size = 12.5f })
                            }
                        }),
                        container({
                            .color = 0xFF0B0F19,
                            .border_radius = BorderRadius::circular(4.0f),
                            .border = Border(0xFF475569, 1.0f),
                            .padding = StyleInsets::symmetric(2.0f, 6.0f),
                            .child = text("Ctrl+K", { .color = 0xFF38BDF8, .font_size = 11.0f, .font_weight = FontWeight::Bold })
                        })
                    }
                })
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this] {
                palette_ctrl_->open();
            }
        });

        auto topbar = container({
            .color = 0xFF0B0F19,
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::symmetric(12.0f, 24.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .children = {
                    logo,
                    search_button
                }
            })
        });

        // ── 2. Metric Stat Cards ──────────────────────────────────────
        auto stat_card = [](std::string title, std::string val, std::string subtitle, Color accent) -> WidgetPtr {
            return container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(0xFF1E293B, 1.0f),
                .height = StyleValue::point(96.0f),
                .padding = StyleInsets::all(16.0f),
                .flex = 1.0f,
                .child = column({
                    .justify_content = Justify::SpaceBetween,
                    .children = {
                        text(title, { .color = 0xFF64748B, .font_size = 11.0f, .font_weight = FontWeight::SemiBold }),
                        text(val, { .color = accent, .font_size = 18.0f, .font_weight = FontWeight::Bold }),
                        text(subtitle, { .color = 0xFF94A3B8, .font_size = 11.0f })
                    }
                })
            });
        };

        auto stats_row = row({
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(96.0f),
            .children = {
                stat_card("COMMAND ENGINE", "Fuzzy Ranked", "Subsequence + Word Bonus", 0xFF38BDF8),
                stat_card("ACTIVE SHORTCUT", "Ctrl+K / ⌘K", "Global Keyboard Listener", 0xFF10B981),
                stat_card("TOTAL EXECUTIONS", std::to_string(total_executions_), "Interactive Studio Session", 0xFFF59E0B),
                stat_card("CURRENT THEME", current_theme_, "Real-Time Switcher", 0xFF8B5CF6)
            }
        });

        // ── 3. Quick Action Buttons ───────────────────────────────────
        auto make_action_btn = [](std::string label, auto on_clk, Color bg_col) -> WidgetPtr {
            return button({
                .child = text(label, { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::SemiBold }),
                .on_pressed = on_clk,
                .normal_color = bg_col,
                .border_radius = 8.0f
            });
        };

        auto actions_bar = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("⚡ QUICK COMMAND PALETTE ACTIONS", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            make_action_btn("🚀 Open Command Palette (Ctrl+K)", [this] { palette_ctrl_->open(); }, 0xFF0284C7),
                            make_action_btn("🔍 Pre-fill 'Git' Query", [this] { palette_ctrl_->setQuery("git"); palette_ctrl_->open(); }, 0xFF0D9488),
                            make_action_btn("🎨 Pre-fill 'Theme' Query", [this] { palette_ctrl_->setQuery("theme"); palette_ctrl_->open(); }, 0xFF7C3AED),
                            make_action_btn("🧹 Clear Recent History", [this] { palette_ctrl_->clearRecent(); logAction("Cleared recent commands history"); }, 0xFF475569)
                        }
                    })
                }
            })
        });

        // ── 4. Live Activity Execution Log ────────────────────────────
        std::vector<WidgetPtr> log_entries;
        for (const auto& log_str : activity_log_) {
            log_entries.push_back(container({
                .color = 0xFF0B0F19,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(0xFF1E293B, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 10.0f),
                .child = text(log_str, { .color = 0xFFCBD5E1, .font_size = 12.0f })
            }));
        }

        auto activity_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .min_height = StyleValue::point(260.0f),
            .padding = StyleInsets::all(16.0f),
            .flex = 1.0f,
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("📜 LIVE ACTIVITY & EXECUTION TELEMETRY", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                            text("Updates in Real-Time", { .color = 0xFF10B981, .font_size = 11.0f })
                        }
                    }),
                    column({
                        .gap = StyleValue::point(6.0f),
                        .children = log_entries
                    })
                }
            })
        });

        // ── 5. Main Viewport Content Stack ────────────────────────────
        auto dashboard_body = container({
            .color = 0xFF080C14,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = {
                    topbar,
                    container({
                        .width = StyleValue::percent(100.0f),
                        .padding = StyleInsets::all(24.0f),
                        .flex = 1.0f,
                        .child = column({
                            .gap = StyleValue::point(16.0f),
                            .width = StyleValue::percent(100.0f),
                            .height = StyleValue::percent(100.0f),
                            .children = {
                                stats_row,
                                actions_bar,
                                activity_box
                            }
                        })
                    })
                }
            })
        });

        // ── 6. CommandPalette Declarative Overlay Wrap ─────────────────
        return CommandPalette {
            .body = dashboard_body,
            .items = buildSampleCommands(),
            .options = {
                .placeholder = "Type a command or search actions...",
                .empty_text = "No commands matched your query",
                .enable_global_shortcut = true,
                .auto_close_on_select = true,
                .on_item_selected = [this](const CommandItem& item) {
                    std::cout << "[CommandPalette] Executed: " << item.title << " (" << item.id << ")\n";
                }
            },
            .controller = palette_ctrl_,
            .initial_open = false
        };
    }
};

class CommandPaletteDemoApp : public StatefulWidget {
public:
    [[nodiscard]] std::unique_ptr<State> createState() override {
        return std::make_unique<CommandPaletteDemoState>();
    }
    [[nodiscard]] std::string_view typeName() const override { return "CommandPaletteDemoApp"; }
};

int main(int argc, char** argv) {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — CommandPalette Studio Showcase Demo\n";
    std::cout << "  Roadmap v0.2.0 | Section 19 Overlay & Popup Extended\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — CommandPalette Studio Showcase Demo";
    config.width       = 1140;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0x0000004D;

    return runApp(std::make_shared<CommandPaletteDemoApp>(), config);
}
