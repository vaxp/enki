/// @file main.cpp
/// @brief ENKI Advanced Modal Dialog Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/dialog.hpp"
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

using namespace enki;

class DialogDemoState : public State {
private:
    std::shared_ptr<DialogController> dialog_ctrl_;
    std::string current_dialog_mode_ = "delete"; // "delete", "success", "invite", "license"
    std::string hud_msg_ = "Click any card button to present an interactive modal dialog.";

    // ── 1. Destructive Delete Dialog Content ──────────────────────
    WidgetPtr buildDeleteContent() {
        return container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(12.0f),
            .child = text("This action cannot be undone. All database snapshots, GPU clusters, and provisioned API keys under 'enki-prod-us-east' will be permanently deleted.", { .color = 0xFFCBD5E1, .font_size = 13.0f })
        });
    }

    // ── 2. Deployment Success Dialog Content ───────────────────────
    WidgetPtr buildSuccessContent() {
        return container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("📦 Bundle Size: 2.4 MB (Brotli compressed)", { .color = 0xFFE2E8F0, .font_size = 12.5f }),
                    text("⚡ Cold Start: 4.2 ms • 60 FPS Skia Compositor", { .color = 0xFF10B981, .font_size = 12.5f }),
                    text("🌐 CDN Distribution: 240 Edge Nodes Worldwide", { .color = 0xFF38BDF8, .font_size = 12.5f })
                }
            })
        });
    }

    // ── 3. Invite Team Member Dialog Content ──────────────────────
    WidgetPtr buildInviteContent() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("Member Email Address:", { .color = 0xFF94A3B8, .font_size = 12.0f, .font_weight = FontWeight::Bold }),
                container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(6.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .width = StyleValue::percent(100.0f),
                    .padding = StyleInsets::symmetric(8.0f, 12.0f),
                    .child = text("alex.developer@enterprise.io", { .color = 0xFFFFFFFF, .font_size = 13.0f })
                }),
                text("Workspace Permission Role:", { .color = 0xFF94A3B8, .font_size = 12.0f, .font_weight = FontWeight::Bold }),
                container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(6.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .width = StyleValue::percent(100.0f),
                    .padding = StyleInsets::symmetric(8.0f, 12.0f),
                    .child = text("⚡ Lead System Architect (Admin)", { .color = 0xFF38BDF8, .font_size = 13.0f })
                })
            }
        });
    }

    // ── 4. License Activation Dialog Content ──────────────────────
    WidgetPtr buildLicenseContent() {
        return column({
            .gap = StyleValue::point(10.0f),
            .children = {
                text("Enter your 25-digit ENKI Enterprise license key below to unlock Skia Vulkan hardware pipelines.", { .color = 0xFFCBD5E1, .font_size = 12.5f }),
                container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(6.0f),
                    .border = Border(0xFFF59E0B, 1.0f),
                    .width = StyleValue::percent(100.0f),
                    .padding = StyleInsets::symmetric(10.0f, 16.0f),
                    .child = text("ENKI-2026-PROX-9482-GOLD", { .color = 0xFFF59E0B, .font_size = 14.0f, .font_weight = FontWeight::Bold })
                })
            }
        });
    }

public:
    void initState() override {
        State::initState();
        dialog_ctrl_ = std::make_shared<DialogController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {
                text("Advanced In-Window Modal Dialog Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                text("In-window stack overlay layer (Category 7. Overlays), 60fps scale-and-fade animation, and rich variants", { .color = 0xFF94A3B8, .font_size = 13.0f })
            }
        });

        // ── Helper to make trigger cards ──────────────────────────────
        auto makeCard = [this](std::string icon, std::string heading, std::string desc,
                              std::string btn_label, Color btn_col, std::string mode) -> WidgetPtr {
            auto gd = std::make_shared<GestureDetector>(container({
                .color = btn_col,
                .border_radius = BorderRadius::circular(6.0f),
                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                .child = text(btn_label, { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::Bold })
            }));
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, mode, heading](const TapUpDetails&) {
                current_dialog_mode_ = mode;
                hud_msg_ = "Opened: " + heading;
                dialog_ctrl_->show();
                setState([] {});
            };

            return container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::point(260.0f),
                .padding = StyleInsets::all(16.0f),
                .child = column({
                    .gap = StyleValue::point(12.0f),
                    .children = {
                        row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text(icon, { .font_size = 20.0f }),
                                text(heading, { .color = 0xFFF1F5F9, .font_size = 14.5f, .font_weight = FontWeight::Bold })
                            }
                        }),
                        text(desc, { .color = 0xFF94A3B8, .font_size = 12.0f }),
                        gd
                    }
                })
            });
        };

        auto card1 = makeCard("🗑️", "Destructive Action", "Delete database cluster with danger confirmation.",
                              "⚠️ Delete Cluster", 0xFFDC2626, "delete");

        auto card2 = makeCard("🚀", "Deployment Ready", "Review production release and Edge statistics.",
                              "✅ View Deployment", 0xFF059669, "success");

        auto card3 = makeCard("👥", "Invite Member", "Add new architect to cloud organization.",
                              "✉️ Invite Member", 0xFF0284C7, "invite");

        auto card4 = makeCard("🔒", "Activate License", "Unlock Enterprise Vulkan GPU pipeline.",
                              "🔑 Enter License", 0xFFD97706, "license");

        // ── Assemble Page Body ────────────────────────────────────────
        auto background_page = container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(24.0f),
                .children = {
                    title_col,
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(14.0f),
                        .children = {card1, card2, card3, card4}
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(1082.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                    })
                }
            })
        });

        // ── Prepare Active Dialog Options & Content ───────────────────
        DialogOptions dialog_opts;
        WidgetPtr active_content;

        if (current_dialog_mode_ == "delete") {
            dialog_opts.type = DialogType::Danger;
            dialog_opts.icon = "⚠️";
            dialog_opts.title = "Permanently Delete Cluster?";
            dialog_opts.subtitle = "Target: enki-prod-us-east (ID: #94829)";
            dialog_opts.width = 480.0f;
            dialog_opts.actions = {
                DialogAction::cancel("Cancel", [this] {
                    hud_msg_ = "Cancelled cluster deletion.";
                    setState([] {});
                }),
                DialogAction::danger("Delete Permanent", [this] {
                    hud_msg_ = "Cluster 'enki-prod-us-east' was permanently deleted.";
                    setState([] {});
                })
            };
            active_content = buildDeleteContent();
        } else if (current_dialog_mode_ == "success") {
            dialog_opts.type = DialogType::Success;
            dialog_opts.icon = "✅";
            dialog_opts.title = "Release Deployed Successfully!";
            dialog_opts.subtitle = "Pipeline #8392 passed all tests in 1.4s";
            dialog_opts.width = 480.0f;
            dialog_opts.actions = {
                DialogAction::primary("Done & Return", [this] {
                    hud_msg_ = "Acknowledged deployment status.";
                    setState([] {});
                })
            };
            active_content = buildSuccessContent();
        } else if (current_dialog_mode_ == "invite") {
            dialog_opts.type = DialogType::Info;
            dialog_opts.icon = "👥";
            dialog_opts.title = "Invite Team Member";
            dialog_opts.subtitle = "Workspace: ENKI Core Architecture Team";
            dialog_opts.width = 480.0f;
            dialog_opts.actions = {
                DialogAction::cancel("Cancel"),
                DialogAction::primary("Send Invitation", [this] {
                    hud_msg_ = "Sent workspace invite to alex.developer@enterprise.io";
                    setState([] {});
                })
            };
            active_content = buildInviteContent();
        } else { // "license"
            dialog_opts.type = DialogType::Warning;
            dialog_opts.icon = "🔒";
            dialog_opts.title = "Activate Enterprise License";
            dialog_opts.subtitle = "Organization: Hyperion Systems Corp";
            dialog_opts.width = 480.0f;
            dialog_opts.actions = {
                DialogAction::cancel("Later"),
                DialogAction::primary("Activate License", [this] {
                    hud_msg_ = "Enterprise License Activated successfully!";
                    setState([] {});
                })
            };
            active_content = buildLicenseContent();
        }

        return dialog({
            .dialog_content = active_content,
            .child = background_page,
            .options = dialog_opts,
            .controller = dialog_ctrl_
        });
    }
};

class DialogDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<DialogDemoState>();
    }
    std::string_view typeName() const override { return "DialogDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Modal Dialog Overlay Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Modal Dialog Overlay Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DialogDemoApp>(), config);
}
