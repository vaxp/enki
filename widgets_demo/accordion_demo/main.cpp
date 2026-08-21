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
        auto t1 = text("🔑 SSH Keys & Access Tokens");
        t1->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto d1 = text("Manage authorized cryptographic public keys for container deployments and remote debug tunnels.");
        d1->fontSize(12.0f).color(0xFF94A3B8);

        auto btn1 = button(text("Generate New API Key"), [this] {
            hud_msg_ = "Action: Initiated API Key generation dialog.";
            setState([] {});
        });

        std::vector<WidgetPtr> items = {t1, d1, btn1};
        auto col = column(items);
        col->gap(StyleValue::point(10.0f));
        return col;
    }

    // ── 2. Section 2 Body: GPU Clusters & Compute ─────────────────
    WidgetPtr buildComputeContent() {
        auto t2 = text("⚡ Vulkan Hardware Acceleration Nodes");
        t2->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto d2 = text("Active cluster: 8x NVIDIA H100 SXM5 with 640GB total VRAM running ENKI Skia compositor.");
        d2->fontSize(12.0f).color(0xFF94A3B8);

        auto stat_txt = text("🟢 Cluster Status: Healthy • 99.98% SLA • 42°C");
        stat_txt->fontSize(12.0f).color(0xFF10B981);

        std::vector<WidgetPtr> items = {t2, d2, stat_txt};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));
        return col;
    }

    // ── 3. Section 3 Body: Database & Storage ─────────────────────
    WidgetPtr buildDatabaseContent() {
        auto t3 = text("💾 PostgreSQL & NVMe High-Availability");
        t3->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto d3 = text("Continuous WAL streaming with automated point-in-time recovery across 3 availability zones.");
        d3->fontSize(12.0f).color(0xFF94A3B8);

        auto btn3 = button(text("Create Manual Snapshot"), [this] {
            hud_msg_ = "Action: Created database snapshot #9482.";
            setState([] {});
        });

        std::vector<WidgetPtr> items = {t3, d3, btn3};
        auto col = column(items);
        col->gap(StyleValue::point(10.0f));
        return col;
    }

    // ── 4. Section 4 Body: Billing & Invoices ─────────────────────
    WidgetPtr buildBillingContent() {
        auto t4 = text("💳 Enterprise Plan Billing Overview");
        t4->fontSize(13.0f).bold().color(0xFFFFFFFF);

        auto d4 = text("Current billing period: Aug 1 - Aug 31, 2026. Estimated usage: $1,420.00 / $5,000.00 credit.");
        d4->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> items = {t4, d4};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));
        return col;
    }

public:
    void initState() override {
        State::initState();
        accordion_ctrl_ = std::make_shared<AccordionController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Accordion Component Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Collapsible disclosure panels (Category 10. Advanced / Data UI), Single/Multiple modes, and 3 visual variants");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Mode & Variant Control Bar ────────────────────────────────
        auto makePill = [](std::string label, bool active, std::function<void()> cb) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(12.0f).color(active ? 0xFFFFFFFF : 0xFF94A3B8);
            if (active) t->bold();

            auto b = container(t);
            b->color(active ? 0xFF0284C7 : 0xFF0F172A)
             .border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(6.0f, 14.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        // Mode Toggles
        auto mode_lbl = text("Expansion Mode:");
        mode_lbl->fontSize(12.5f).bold().color(0xFFCBD5E1);

        auto pill_single = makePill("Single Mode", current_mode_ == AccordionMode::Single, [this] {
            current_mode_ = AccordionMode::Single;
            hud_msg_ = "Switched to Single Expansion Mode.";
            setState([] {});
        });

        auto pill_multi = makePill("Multiple Mode", current_mode_ == AccordionMode::Multiple, [this] {
            current_mode_ = AccordionMode::Multiple;
            hud_msg_ = "Switched to Multiple Expansion Mode.";
            setState([] {});
        });

        std::vector<WidgetPtr> mode_items = {mode_lbl, pill_single, pill_multi};
        auto mode_row = row(mode_items);
        mode_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        // Variant Toggles
        auto var_lbl = text("Visual Variant:");
        var_lbl->fontSize(12.5f).bold().color(0xFFCBD5E1);

        auto pill_bordered = makePill("Bordered Card", current_variant_ == AccordionVariant::Bordered, [this] {
            current_variant_ = AccordionVariant::Bordered;
            hud_msg_ = "Switched to Bordered Card variant.";
            setState([] {});
        });

        auto pill_sep = makePill("Separated Cards", current_variant_ == AccordionVariant::Separated, [this] {
            current_variant_ = AccordionVariant::Separated;
            hud_msg_ = "Switched to Separated Cards variant.";
            setState([] {});
        });

        auto pill_flush = makePill("Flush Minimal", current_variant_ == AccordionVariant::Flush, [this] {
            current_variant_ = AccordionVariant::Flush;
            hud_msg_ = "Switched to Flush Minimal variant.";
            setState([] {});
        });

        std::vector<WidgetPtr> var_items = {var_lbl, pill_bordered, pill_sep, pill_flush};
        auto var_row = row(var_items);
        var_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> controls_row_items = {mode_row, var_row};
        auto controls_row = row(controls_row_items);
        controls_row->justifyContent(Justify::SpaceBetween)
                    .alignItems(Align::Center)
                    .width(820.0f);

        // ── Programmatic Actions (Expand/Collapse All) ────────────────
        auto btn_exp_all = button(text("Expand All"), [this] {
            accordion_ctrl_->expandAll();
            hud_msg_ = "Action: Expanded all accordion sections.";
            setState([] {});
        });

        auto btn_col_all = button(text("Collapse All"), [this] {
            accordion_ctrl_->collapseAll();
            hud_msg_ = "Action: Collapsed all accordion sections.";
            setState([] {});
        });

        auto btn_toggle_gpu = button(text("Toggle GPU Section"), [this] {
            accordion_ctrl_->toggle("sec_gpu");
            hud_msg_ = "Action: Toggled GPU Compute section.";
            setState([] {});
        });

        std::vector<WidgetPtr> prog_items = {btn_exp_all, btn_col_all, btn_toggle_gpu};
        auto prog_row = row(prog_items);
        prog_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // ── Assemble Accordion Items ──────────────────────────────────
        auto it1 = AccordionItem("sec_iam", "Security & IAM Access", buildSecurityContent(),
                                 "🛡️", "API tokens and cryptographic keys", true)
            .setBadge("Active", 0x2E10B981, 0xFF10B981);

        auto it2 = AccordionItem("sec_gpu", "GPU Compute & AI Clusters", buildComputeContent(),
                                 "⚡", "8x NVIDIA H100 SXM5 nodes")
            .setBadge("PRO", 0x2EF59E0B, 0xFFF59E0B);

        auto it3 = AccordionItem("sec_db", "Database & High-Availability Storage", buildDatabaseContent(),
                                 "💾", "PostgreSQL WAL streaming");

        auto it4 = AccordionItem("sec_bill", "Billing & Usage Invoices", buildBillingContent(),
                                 "💳", "Monthly resource allocation");

        auto it5 = AccordionItem("sec_legacy", "Legacy v1 Compatibility (Deprecated)", container(),
                                 "🔒", "Requires enterprise permission")
            .setDisabled(true)
            .setBadge("Locked", 0x2EEF4444, 0xFFEF4444);

        std::vector<AccordionItem> accordion_items = {it1, it2, it3, it4, it5};

        AccordionProps opts;
        opts.mode = current_mode_;
        opts.variant = current_variant_;
        opts.on_toggle = [this](const std::string& id, bool exp) {
            hud_msg_ = "Section [" + id + "] is now " + (exp ? "EXPANDED" : "COLLAPSED");
            setState([] {});
        };

        opts.items = accordion_items;
        opts.controller = accordion_ctrl_;
        auto acc_widget = accordion(opts);

        auto acc_wrapper = container(acc_widget);
        acc_wrapper->width(820.0f);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(820.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items;
        page_items.push_back(title_col);
        page_items.push_back(controls_row);
        page_items.push_back(prog_row);
        page_items.push_back(acc_wrapper);
        page_items.push_back(hud_box);
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
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
