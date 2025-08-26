/// @file main.cpp
/// @brief ENKI Advanced Accordion Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/accordion.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class AccordionDemoState : public State {
private:
    std::shared_ptr<AccordionController> accordion_ctrl_;
    AccordionMode current_mode_ = AccordionMode::Single;
    AccordionVariant current_variant_ = AccordionVariant::Bordered;
    std::string hud_msg_ = "Click any accordion header to toggle expansion.";

    // ── 1. Section 1 Body: Security & IAM ─────────────────────────
    WidgetPtr buildSecurityContent() {
        return column({
            .gap = StyleValue::point(10.0f),
            .children = {
                text("🔑 SSH Keys & Access Tokens", {
                    .color = 0xFFFFFFFF,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Manage authorized cryptographic public keys for container deployments and remote debug tunnels.", {
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                }),
                button(text("Generate New API Key"), [this] {
                    hud_msg_ = "Action: Initiated API Key generation dialog.";
                    setState([] {});
                }),
            },
        });
    }

    // ── 2. Section 2 Body: GPU Clusters & Compute ─────────────────
    WidgetPtr buildComputeContent() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("⚡ Vulkan Hardware Acceleration Nodes", {
                    .color = 0xFFFFFFFF,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Active cluster: 8x NVIDIA H100 SXM5 with 640GB total VRAM running ENKI Skia compositor.", {
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                }),
                text("🟢 Cluster Status: Healthy • 99.98% SLA • 42°C", {
                    .color = 0xFF10B981,
                    .font_size = 12.0f,
                }),
            },
        });
    }

    // ── 3. Section 3 Body: Database & Storage ─────────────────────
    WidgetPtr buildDatabaseContent() {
        return column({
            .gap = StyleValue::point(10.0f),
            .children = {
                text("💾 PostgreSQL & NVMe High-Availability", {
                    .color = 0xFFFFFFFF,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Continuous WAL streaming with automated point-in-time recovery across 3 availability zones.", {
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                }),
                button(text("Create Manual Snapshot"), [this] {
                    hud_msg_ = "Action: Created database snapshot #9482.";
                    setState([] {});
                }),
            },
        });
    }

    // ── 4. Section 4 Body: Billing & Invoices ─────────────────────
    WidgetPtr buildBillingContent() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("💳 Enterprise Plan Billing Overview", {
                    .color = 0xFFFFFFFF,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Current billing period: Aug 1 - Aug 31, 2026. Estimated usage: $1,420.00 / $5,000.00 credit.", {
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                }),
            },
        });
    }

public:
    void initState() override {
        State::initState();
        accordion_ctrl_ = std::make_shared<AccordionController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {
                text("Advanced Accordion Component Suite", {
                    .color = 0xFFFFFFFF,
                    .font_size = 22.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Collapsible disclosure panels (Category 10. Advanced / Data UI), Single/Multiple modes, and 3 visual variants", {
                    .color = 0xFF94A3B8,
                    .font_size = 13.0f,
                }),
            },
        });

        // ── Mode & Variant Control Bar ────────────────────────────────
        auto makePill = [](std::string label, bool active, std::function<void()> cb) -> WidgetPtr {
            auto b = container({
                .color = active ? 0xFF0284C7 : 0xFF0F172A,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 14.0f),
                .child = text(std::move(label), {
                    .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
                }),
            });

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        // Mode Toggles
        auto mode_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                text("Expansion Mode:", {
                    .color = 0xFFCBD5E1,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold,
                }),
                makePill("Single Mode", current_mode_ == AccordionMode::Single, [this] {
                    current_mode_ = AccordionMode::Single;
                    hud_msg_ = "Switched to Single Expansion Mode.";
                    setState([] {});
                }),
                makePill("Multiple Mode", current_mode_ == AccordionMode::Multiple, [this] {
                    current_mode_ = AccordionMode::Multiple;
                    hud_msg_ = "Switched to Multiple Expansion Mode.";
                    setState([] {});
                }),
            },
        });

        // Variant Toggles
        auto var_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                text("Visual Variant:", {
                    .color = 0xFFCBD5E1,
                    .font_size = 12.5f,
                    .font_weight = FontWeight::Bold,
                }),
                makePill("Bordered Card", current_variant_ == AccordionVariant::Bordered, [this] {
                    current_variant_ = AccordionVariant::Bordered;
                    hud_msg_ = "Switched to Bordered Card variant.";
                    setState([] {});
                }),
                makePill("Separated Cards", current_variant_ == AccordionVariant::Separated, [this] {
                    current_variant_ = AccordionVariant::Separated;
                    hud_msg_ = "Switched to Separated Cards variant.";
                    setState([] {});
                }),
                makePill("Flush Minimal", current_variant_ == AccordionVariant::Flush, [this] {
                    current_variant_ = AccordionVariant::Flush;
                    hud_msg_ = "Switched to Flush Minimal variant.";
                    setState([] {});
                }),
            },
        });

        auto controls_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::point(820.0f),
            .children = {mode_row, var_row},
        });

        // ── Programmatic Actions (Expand/Collapse All) ────────────────
        auto prog_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                button(text("Expand All"), [this] {
                    accordion_ctrl_->expandAll();
                    hud_msg_ = "Action: Expanded all accordion sections.";
                    setState([] {});
                }),
                button(text("Collapse All"), [this] {
                    accordion_ctrl_->collapseAll();
                    hud_msg_ = "Action: Collapsed all accordion sections.";
                    setState([] {});
                }),
                button(text("Toggle GPU Section"), [this] {
                    accordion_ctrl_->toggle("sec_gpu");
                    hud_msg_ = "Action: Toggled GPU Compute section.";
                    setState([] {});
                }),
            },
        });

        // ── Assemble Accordion Items ──────────────────────────────────
        std::vector<AccordionItem> accordion_items = {
            AccordionItem {
                .id = "sec_iam",
                .title = "Security & IAM Access",
                .subtitle = "API tokens and cryptographic keys",
                .icon = "🛡️",
                .badge_label = "Active",
                .badge_bg = 0x2E10B981,
                .badge_fg = 0xFF10B981,
                .content = buildSecurityContent(),
                .is_initially_expanded = true,
            },
            AccordionItem {
                .id = "sec_gpu",
                .title = "GPU Compute & AI Clusters",
                .subtitle = "8x NVIDIA H100 SXM5 nodes",
                .icon = "⚡",
                .badge_label = "PRO",
                .badge_bg = 0x2EF59E0B,
                .badge_fg = 0xFFF59E0B,
                .content = buildComputeContent(),
            },
            AccordionItem {
                .id = "sec_db",
                .title = "Database & High-Availability Storage",
                .subtitle = "PostgreSQL WAL streaming",
                .icon = "💾",
                .content = buildDatabaseContent(),
            },
            AccordionItem {
                .id = "sec_bill",
                .title = "Billing & Usage Invoices",
                .subtitle = "Monthly resource allocation",
                .icon = "💳",
                .content = buildBillingContent(),
            },
            AccordionItem {
                .id = "sec_legacy",
                .title = "Legacy v1 Compatibility (Deprecated)",
                .subtitle = "Requires enterprise permission",
                .icon = "🔒",
                .badge_label = "Locked",
                .badge_bg = 0x2EEF4444,
                .badge_fg = 0xFFEF4444,
                .content = container(),
                .is_disabled = true,
            },
        };

        auto acc_wrapper = container({
            .width = StyleValue::point(820.0f),
            .child = Accordion {
                .items = std::move(accordion_items),
                .controller = accordion_ctrl_,
                .mode = current_mode_,
                .variant = current_variant_,
                .on_toggle = [this](const std::string& id, bool exp) {
                    hud_msg_ = "Section [" + id + "] is now " + (exp ? "EXPANDED" : "COLLAPSED");
                    setState([] {});
                },
            },
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(820.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = {
                    text("💡 " + hud_msg_, {
                        .color = 0xFF38BDF8,
                        .font_size = 12.5f,
                    }),
                },
            }),
        });

        // ── Assemble Page Body ────────────────────────────────────────
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    title_col,
                    controls_row,
                    prog_row,
                    acc_wrapper,
                    hud_box,
                },
            }),
        });
    }
};

class AccordionDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<AccordionDemoState>();
    }
    std::string_view typeName() const override { return "AccordionDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Accordion Component Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Accordion Component Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<AccordionDemoApp>(), config);
}
