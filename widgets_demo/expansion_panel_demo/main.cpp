/// @file main.cpp
/// @brief ENKI Advanced ExpansionPanel Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/expansion_panel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ExpansionPanelDemoState : public State {
private:
    std::shared_ptr<ExpansionPanelController> panel_ctrl_;
    bool is_radio_mode_ = true;
    std::string hud_msg_ = "Click any step panel header to expand, or click [Next Step] in the footer action bar.";

    // ── Step 1 Body: Compute & GPU ────────────────────────────────
    WidgetPtr buildStep1Body() {
        auto t = text("⚡ Node Sizing & Hardware Acceleration");
        t->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto d = text("Allocate compute instances for real-time Vulkan Skia compositing nodes.");
        d->fontSize(12.0f).color(0xFF94A3B8);

        auto spec = text("Selected: 16 vCPUs • 64GB ECC RAM • 1x NVIDIA H100 GPU");
        spec->fontSize(12.5f).bold().color(0xFF38BDF8);

        std::vector<WidgetPtr> items = {t, d, spec};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));
        return col;
    }

    // ── Step 2 Body: NVMe Storage ─────────────────────────────────
    WidgetPtr buildStep2Body() {
        auto t = text("💾 High-IOPS Persistent Storage");
        t->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto d = text("Configure NVMe direct-attached volumes with automated snapshots.");
        d->fontSize(12.0f).color(0xFF94A3B8);

        auto spec = text("Selected: 2TB NVMe RAID-10 • 120,000 IOPS • Multi-AZ Replication");
        spec->fontSize(12.5f).bold().color(0xFF10B981);

        std::vector<WidgetPtr> items = {t, d, spec};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));
        return col;
    }

    // ── Step 3 Body: Security & IAM ───────────────────────────────
    WidgetPtr buildStep3Body() {
        auto t = text("🛡️ Cryptographic Key Vault & Zero-Trust");
        t->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto d = text("Generate TLS 1.3 mutual authentication certs and enclave hardware keys.");
        d->fontSize(12.0f).color(0xFF94A3B8);

        auto spec = text("Selected: Ed25519 Hardware Enclave • Automatic 90-Day Rotation");
        spec->fontSize(12.5f).bold().color(0xFFF59E0B);

        std::vector<WidgetPtr> items = {t, d, spec};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));
        return col;
    }

public:
    void initState() override {
        State::initState();
        panel_ctrl_ = std::make_shared<ExpansionPanelController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced ExpansionPanel Component Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Multi-step configuration wizard (Category 10. Advanced / Data UI), Radio & Multi modes, and footer action bars");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Control Bar ───────────────────────────────────────────────
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

        auto pill_radio = makePill("Radio Mode (1-Step)", is_radio_mode_, [this] {
            is_radio_mode_ = true;
            hud_msg_ = "Switched to Radio Mode (Expanding one panel collapses previous ones).";
            setState([] {});
        });

        auto pill_multi = makePill("Multi-Panel Mode", !is_radio_mode_, [this] {
            is_radio_mode_ = false;
            hud_msg_ = "Switched to Multi-Panel Mode (Multiple panels can stay open).";
            setState([] {});
        });

        auto btn_exp_all = button(text("Expand All"), [this] {
            panel_ctrl_->expandAll();
            hud_msg_ = "Action: Expanded all available panels.";
            setState([] {});
        });

        auto btn_col_all = button(text("Collapse All"), [this] {
            panel_ctrl_->collapseAll();
            hud_msg_ = "Action: Collapsed all panels.";
            setState([] {});
        });

        std::vector<WidgetPtr> ctrl_items = {pill_radio, pill_multi, btn_exp_all, btn_col_all};
        auto ctrl_row = row(ctrl_items);
        ctrl_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // ── 4 Panel Step Items ────────────────────────────────────────
        // Step 1
        auto p1_btn_next = button(text("Save & Continue ➔"), [this] {
            panel_ctrl_->expand(1);
            hud_msg_ = "Step 1 Completed: Proceeded to Step 2 (Storage).";
            setState([] {});
        });
        auto p1 = ExpansionPanelItem("step_1", "1. Compute & GPU Acceleration", buildStep1Body(),
                                     "1", "16 vCPUs • 1x NVIDIA H100", true)
            .setBadge("Completed", 0x2E10B981, 0xFF10B981)
            .setFooterActions({p1_btn_next});

        // Step 2
        auto p2_btn_back = button(text("◀ Back"), [this] {
            panel_ctrl_->expand(0);
        });
        auto p2_btn_next = button(text("Save & Continue ➔"), [this] {
            panel_ctrl_->expand(2);
            hud_msg_ = "Step 2 Completed: Proceeded to Step 3 (Security).";
            setState([] {});
        });
        auto p2 = ExpansionPanelItem("step_2", "2. High-Performance NVMe Storage", buildStep2Body(),
                                     "2", "2TB NVMe RAID-10")
            .setBadge("In Progress", 0x2EF59E0B, 0xFFF59E0B)
            .setFooterActions({p2_btn_back, p2_btn_next});

        // Step 3
        auto p3_btn_back = button(text("◀ Back"), [this] {
            panel_ctrl_->expand(1);
        });
        auto p3_btn_deploy = button(text("🚀 Finalize & Deploy Cluster"), [this] {
            hud_msg_ = "Action: Initiated Cloud Cluster Deployment!";
            setState([] {});
        });
        auto p3 = ExpansionPanelItem("step_3", "3. Cryptographic Security & IAM", buildStep3Body(),
                                     "3", "Ed25519 Enclave Keys")
            .setBadge("Pending", 0x2E38BDF8, 0xFF38BDF8)
            .setFooterActions({p3_btn_back, p3_btn_deploy});

        // Step 4 (Locked / Disabled)
        auto p4 = ExpansionPanelItem("step_4", "4. Post-Deployment Telemetry (Locked)", container(),
                                     "🔒", "Unlocked after cluster provisioning")
            .setDisabled(true)
            .setBadge("Locked", 0x2EEF4444, 0xFFEF4444);

        std::vector<ExpansionPanelItem> panel_list = {p1, p2, p3, p4};

        ExpansionPanelOptions panel_opts;
        panel_opts.is_radio_mode = is_radio_mode_;
        panel_opts.on_panel_toggled = [this](int idx, bool exp) {
            hud_msg_ = "Panel #" + std::to_string(idx + 1) + " is now " + (exp ? "EXPANDED" : "COLLAPSED");
            setState([] {});
        };

        auto exp_list_widget = expansionPanelList(panel_list, panel_opts, panel_ctrl_);

        auto exp_wrapper = container(exp_list_widget);
        exp_wrapper->width(820.0f);

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
        std::vector<WidgetPtr> page_items = {title_col, ctrl_row, exp_wrapper, hud_box};
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

class ExpansionPanelDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ExpansionPanelDemoState>();
    }
    std::string_view typeName() const override { return "ExpansionPanelDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ExpansionPanel Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ExpansionPanel Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ExpansionPanelDemoApp>(), config);
}
