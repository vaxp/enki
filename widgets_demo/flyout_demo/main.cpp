/// @file main.cpp
/// @brief ENKI Shell Flyout & Control Center Interactive Showcase.
/// Demonstrates the new ShellFlyout subsystem for Control Center and Notification Drawers.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flyout.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ── Interactive Quick Toggle Card ──────────────────────────────

class QuickToggle : public StatefulWidget {
public:
    std::string title;
    std::string subtitle;
    bool initial_active;

    QuickToggle(std::string title, std::string subtitle, bool active = false)
        : title(std::move(title)), subtitle(std::move(subtitle)), initial_active(active) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "QuickToggle"; }
};

class QuickToggleState : public State {
private:
    bool active_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const QuickToggle*>(widget());
        if (w) active_ = w->initial_active;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const QuickToggle*>(widget());

        Color bg = active_ ? 0xFF3B82F6 : 0xFF272C35;
        Color text_col = active_ ? 0xFFFFFFFF : 0xFFE2E8F0;
        Color sub_col = active_ ? 0xFFDBEAFE : 0xFF94A3B8;

        auto card = container({
            .color = bg,
            .border_radius = BorderRadius::circular(12.0f),
            .width = 160.0f,
            .height = 54.0f,
            .padding = StyleInsets::all(10.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Start,
                .children = {
                    text({
                        .text = w ? w->title : "",
                        .color = text_col,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text({
                        .text = active_ ? "On" : (w ? w->subtitle : ""),
                        .color = sub_col,
                        .font_size = 11.0f,
                    }),
                },
            }),
        });

        GestureDetectorProps props;
        props.child = card;
        props.on_tap = [this]() {
            setState([this]() {
                active_ = !active_;
            });
        };

        return gestureDetector(props);
    }
};

std::unique_ptr<State> QuickToggle::createState() {
    return std::make_unique<QuickToggleState>();
}

// ── Control Center View Builder ────────────────────────────────

static WidgetPtr buildControlCenterView(BuildContext&, std::shared_ptr<FlyoutHandle> handle) {
    return column({
        .align_items = Align::Stretch,
        .children = {
            // Header
            row({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    text({
                        .text = "Control Center",
                        .color = 0xFFFFFFFF,
                        .font_size = 16.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text({
                        .text = "ENKI Shell",
                        .color = 0xFF38BDF8,
                        .font_size = 12.0f,
                        .font_weight = FontWeight::Medium,
                    }),
                },
            }),

            container({ .height = 16.0f }),

            // Quick Settings Grid
            row({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    std::make_shared<QuickToggle>("Wi-Fi", "Connected", true),
                    std::make_shared<QuickToggle>("Bluetooth", "Available", true),
                },
            }),

            container({ .height = 10.0f }),

            row({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    std::make_shared<QuickToggle>("Do Not Disturb", "Off", false),
                    std::make_shared<QuickToggle>("Night Light", "Schedule", false),
                },
            }),

            container({ .height = 16.0f }),

            // Audio Slider Card
            container({
                .color = 0xFF272C35,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::all(12.0f),
                .child = column({
                    .align_items = Align::Start,
                    .children = {
                        text({
                            .text = "Master Volume (PulseAudio)",
                            .color = 0xFFE2E8F0,
                            .font_size = 12.0f,
                            .font_weight = FontWeight::Medium,
                        }),
                        container({ .height = 8.0f }),
                        // Volume progress bar simulation
                        container({
                            .color = 0xFF1E222A,
                            .border_radius = BorderRadius::circular(5.0f),
                            .height = 10.0f,
                            .child = row({
                                .children = {
                                    container({
                                        .color = 0xFF38BDF8,
                                        .border_radius = BorderRadius::circular(5.0f),
                                        .width = 220.0f,
                                        .height = 10.0f,
                                    }),
                                },
                            }),
                        }),
                    },
                }),
            }),

            container({ .height = 10.0f }),

            // Brightness Card
            container({
                .color = 0xFF272C35,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::all(12.0f),
                .child = column({
                    .align_items = Align::Start,
                    .children = {
                        text({
                            .text = "Display Brightness",
                            .color = 0xFFE2E8F0,
                            .font_size = 12.0f,
                            .font_weight = FontWeight::Medium,
                        }),
                        container({ .height = 8.0f }),
                        container({
                            .color = 0xFF1E222A,
                            .border_radius = BorderRadius::circular(5.0f),
                            .height = 10.0f,
                            .child = row({
                                .children = {
                                    container({
                                        .color = 0xFFFBBF24,
                                        .border_radius = BorderRadius::circular(5.0f),
                                        .width = 180.0f,
                                        .height = 10.0f,
                                    }),
                                },
                            }),
                        }),
                    },
                }),
            }),

            container({ .height = 16.0f }),

            // Close Button
            button({
                .child = text({
                    .text = "Close Panel",
                    .color = 0xFFE2E8F0,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::SemiBold,
                }),
                .on_pressed = [handle]() {
                    if (handle) handle->close();
                },
                .normal_color = 0xFF334155,
                .hover_color = 0xFF475569,
            }),
        },
    });
}

// ── Notification Center View Builder ───────────────────────────

static WidgetPtr buildNotificationCenterView(BuildContext&, std::shared_ptr<FlyoutHandle> handle) {
    return column({
        .align_items = Align::Stretch,
        .children = {
            row({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    text({
                        .text = "Notifications",
                        .color = 0xFFFFFFFF,
                        .font_size = 16.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text({
                        .text = "Clear All",
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f,
                    }),
                },
            }),

            container({ .height = 14.0f }),

            // Notification Card 1
            container({
                .color = 0xFF272C35,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::all(12.0f),
                .child = column({
                    .align_items = Align::Start,
                    .children = {
                        text({
                            .text = "AetherShell Update Available",
                            .color = 0xFFFFFFFF,
                            .font_size = 13.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        container({ .height = 4.0f }),
                        text({
                            .text = "ENKI reactive shell v0.2.0 is now compiled and ready for deployment.",
                            .color = 0xFF94A3B8,
                            .font_size = 12.0f,
                        }),
                    },
                }),
            }),

            container({ .height = 10.0f }),

            // Notification Card 2
            container({
                .color = 0xFF272C35,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::all(12.0f),
                .child = column({
                    .align_items = Align::Start,
                    .children = {
                        text({
                            .text = "Battery Status",
                            .color = 0xFFFFFFFF,
                            .font_size = 13.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        container({ .height = 4.0f }),
                        text({
                            .text = "Power adapter plugged in. Charging (98%).",
                            .color = 0xFF94A3B8,
                            .font_size = 12.0f,
                        }),
                    },
                }),
            }),

            container({ .height = 16.0f }),

            button({
                .child = text({
                    .text = "Dismiss",
                    .color = 0xFFE2E8F0,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::SemiBold,
                }),
                .on_pressed = [handle]() {
                    if (handle) handle->close();
                },
                .normal_color = 0xFF334155,
                .hover_color = 0xFF475569,
            }),
        },
    });
}

// ── Main Top Bar Widget ────────────────────────────────────────

static WidgetPtr createTopPanelBar() {
    auto cc_button = container({
        .color = 0xFF272C35,
        .border_radius = BorderRadius::circular(8.0f),
        .padding = StyleInsets::symmetric(8.0f, 14.0f),
        .child = text({
            .text = "⚙ Quick Settings",
            .color = 0xFFFFFFFF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        }),
    });

    auto notif_button = container({
        .color = 0xFF272C35,
        .border_radius = BorderRadius::circular(8.0f),
        .padding = StyleInsets::symmetric(8.0f, 14.0f),
        .child = text({
            .text = "🔔 Notifications (2)",
            .color = 0xFFFFFFFF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        }),
    });

    const std::string pixel_art_border_shader = R"(
        uniform float time;
        uniform vec2 resolution;

        vec4 main(vec2 fragCoord) {
            float p = 3.0; // Retro pixel size
            vec2 g = floor(fragCoord / p);
            vec2 sz = floor(resolution / p);

            float dL = g.x;
            float dR = sz.x - 1.0 - g.x;
            float dT = g.y;
            float dB = sz.y - 1.0 - g.y;

            float dX = min(dL, dR);
            float dY = min(dT, dB);

            // Palette extracted directly from the user's PNG image
            vec4 cOutline = vec4(0.18, 0.16, 0.19, 1.0); // #2E2930 Charcoal dark border
            vec4 cDark    = vec4(0.32, 0.29, 0.34, 1.0); // #524A57 Deep iron shadow
            vec4 cMid     = vec4(0.48, 0.45, 0.50, 1.0); // #7A7380 Stone/iron body
            vec4 cLight   = vec4(0.68, 0.65, 0.71, 1.0); // #AEA6B5 Highlight stone

            // ── 1. Corner Rivet Blocks (5x5 pixels) ──
            if (dX < 5.0 && dY < 5.0) {
                if (dX == 0.0 && dY == 0.0) return vec4(0.0); // Outer diagonal corner cutout
                if (dX == 0.0 || dY == 0.0 || dX == 4.0 || dY == 4.0) {
                    return cOutline;
                }
                if (dX == 2.0 && dY == 2.0) {
                    return cDark; // Center rivet hole
                }
                if (dY <= 2.0 && dX <= 2.0) {
                    return cLight; // Specular highlight
                }
                return cMid;
            }

            // ── 2. Edge Rails with Staggered Cog Teeth ──
            float depth = min(dX, dY);
            if (depth > 4.0) {
                return vec4(0.0); // Inner transparent area
            }

            float along = (dX < dY) ? g.y : g.x;
            float modStep = mod(along, 4.0);

            // Row 0: Outer protruding teeth
            if (depth == 0.0) {
                if (modStep == 1.0 || modStep == 2.0) return cOutline;
                return vec4(0.0);
            }

            // Row 1: Outer rail line
            if (depth == 1.0) {
                if (modStep == 1.0 || modStep == 2.0) return cLight;
                return cOutline;
            }

            // Row 2: Center rail track
            if (depth == 2.0) {
                if (modStep == 0.0 || modStep == 3.0) return cDark;
                return cMid;
            }

            // Row 3: Inner rail line (staggered)
            if (depth == 3.0) {
                if (modStep == 0.0 || modStep == 3.0) return cOutline;
                return cDark;
            }

            // Row 4: Inner protruding teeth
            if (depth == 4.0) {
                if (modStep == 0.0 || modStep == 3.0) return cOutline;
                return vec4(0.0);
            }

            return cOutline;
        }
    )";

    // Wrap buttons in Flyout widgets
    auto cc_flyout = flyout(Flyout{
        .child = cc_button,
        .builder = buildControlCenterView,
        .options = FlyoutOptions{
            .width = 380,
            .height = 500,
            .auto_dismiss = true,
            .background_color = 0xF5131117,
            .border_width = 15.0f,
            .border_shader = pixel_art_border_shader,
            .box_shadow = {
                BoxShadow::glow(0x407A7380, 20.0f),
                BoxShadow::standard(0x80000000, 15.0f, 8.0f)
            },
            .padding = EdgeInsets::all(20.0f),
        },
    });

    auto notif_flyout = flyout(Flyout{
        .child = notif_button,
        .builder = buildNotificationCenterView,
        .options = FlyoutOptions{
            .width = 360,
            .height = 360,
            .auto_dismiss = true,
            .border_color = 0xFF38BDF8,
            .border_width = 1.0f,
            .box_shadow = { BoxShadow::standard(0x80000000, 20.0f, 8.0f) },
        },
    });

    return container({
        .color = 0xFF0F1218,
        .border = Border(0xFF272C35, 1.0f),
        .height = 48.0f,
        .padding = StyleInsets::symmetric(6.0f, 16.0f),
        .child = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                // Left: System branding
                text({
                    .text = "⚡ ENKI Shell Panel",
                    .color = 0xFF38BDF8,
                    .font_size = 14.0f,
                    .font_weight = FontWeight::Bold,
                }),

                // Center: Status
                text({
                    .text = "Desktop Layer: Top / Flyouts Enabled",
                    .color = 0xFF64748B,
                    .font_size = 12.0f,
                }),

                // Right: Flyout Triggers
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        notif_flyout,
                        cc_flyout,
                    },
                }),
            },
        }),
    });
}

int main() {
    AppConfig config;
    config.title = "ENKI Shell Flyout Showcase";
    config.width = 1100;
    config.height = 700;
    config.vsync = true;

    auto root = column({
        .align_items = Align::Stretch,
        .children = {
            createTopPanelBar(),
            container({
                .padding = StyleInsets::all(32.0f),
                .child = column({
                    .align_items = Align::Start,
                    .children = {
                        text({
                            .text = "Shell Flyout & Drawer Subsystem",
                            .color = 0xFFFFFFFF,
                            .font_size = 28.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        container({ .height = 12.0f }),
                        text({
                            .text = "This showcase demonstrates the native ShellFlyout windowing subsystem.\n"
                                    "Unlike standard popups (xdg_popup), ShellFlyout creates standalone native\n"
                                    "system overlay surfaces that automatically align to the trigger button in the\n"
                                    "panel, handle click-outside auto-dismiss, and support nested popups without grab conflicts.",
                            .color = 0xFF94A3B8,
                            .font_size = 14.0f,
                        }),
                    },
                }),
            }),
        },
    });

    return runApp(root, config);
}
