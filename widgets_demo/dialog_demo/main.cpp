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
        auto warning_txt = text("This action cannot be undone. All database snapshots, GPU clusters, and provisioned API keys under 'enki-prod-us-east' will be permanently deleted.");
        warning_txt->fontSize(13.0f).color(0xFFCBD5E1);

        auto box = container(warning_txt);
        box->color(0xFF0F172A)
           .borderRadius(6.0f)
           .border(0xFF334155, 1.0f)
           .paddingAll(12.0f)
           .width(StyleValue::percent(100.0f));
        return box;
    }

    // ── 2. Deployment Success Dialog Content ───────────────────────
    WidgetPtr buildSuccessContent() {
        auto stat1_lbl = text("📦 Bundle Size: 2.4 MB (Brotli compressed)");
        stat1_lbl->fontSize(12.5f).color(0xFFE2E8F0);

        auto stat2_lbl = text("⚡ Cold Start: 4.2 ms • 60 FPS Skia Compositor");
        stat2_lbl->fontSize(12.5f).color(0xFF10B981);

        auto stat3_lbl = text("🌐 CDN Distribution: 240 Edge Nodes Worldwide");
        stat3_lbl->fontSize(12.5f).color(0xFF38BDF8);

        std::vector<WidgetPtr> items = {stat1_lbl, stat2_lbl, stat3_lbl};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));

        auto box = container(col);
        box->color(0xFF0F172A)
           .borderRadius(6.0f)
           .border(0xFF334155, 1.0f)
           .paddingAll(14.0f)
           .width(StyleValue::percent(100.0f));
        return box;
    }

    // ── 3. Invite Team Member Dialog Content ──────────────────────
    WidgetPtr buildInviteContent() {
        auto email_label = text("Member Email Address:");
        email_label->fontSize(12.0f).bold().color(0xFF94A3B8);

        auto email_val = text("alex.developer@enterprise.io");
        email_val->fontSize(13.0f).color(0xFFFFFFFF);
        auto email_box = container(email_val);
        email_box->color(0xFF0F172A)
                 .borderRadius(6.0f)
                 .border(0xFF334155, 1.0f)
                 .paddingSymmetric(8.0f, 12.0f)
                 .width(StyleValue::percent(100.0f));

        auto role_label = text("Workspace Permission Role:");
        role_label->fontSize(12.0f).bold().color(0xFF94A3B8);

        auto role_val = text("⚡ Lead System Architect (Admin)");
        role_val->fontSize(13.0f).color(0xFF38BDF8);
        auto role_box = container(role_val);
        role_box->color(0xFF0F172A)
                .borderRadius(6.0f)
                .border(0xFF334155, 1.0f)
                .paddingSymmetric(8.0f, 12.0f)
                .width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> items = {email_label, email_box, role_label, role_box};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));

        return col;
    }

    // ── 4. License Activation Dialog Content ──────────────────────
    WidgetPtr buildLicenseContent() {
        auto lic_txt = text("Enter your 25-digit ENKI Enterprise license key below to unlock Skia Vulkan hardware pipelines.");
        lic_txt->fontSize(12.5f).color(0xFFCBD5E1);

        auto key_txt = text("ENKI-2026-PROX-9482-GOLD");
        key_txt->fontSize(14.0f).bold().color(0xFFF59E0B);
        auto key_box = container(key_txt);
        key_box->color(0xFF0F172A)
               .borderRadius(6.0f)
               .border(0xFFF59E0B, 1.0f)
               .paddingSymmetric(10.0f, 16.0f)
               .width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> items = {lic_txt, key_box};
        auto col = column(items);
        col->gap(StyleValue::point(10.0f));
        return col;
    }

public:
    void initState() override {
        State::initState();
        dialog_ctrl_ = std::make_shared<DialogController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced In-Window Modal Dialog Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("In-window stack overlay layer (Category 7. Overlays), 60fps scale-and-fade animation, and rich variants");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Helper to make trigger cards ──────────────────────────────
        auto makeCard = [this](std::string icon, std::string heading, std::string desc,
                              std::string btn_label, Color btn_col, std::string mode) -> WidgetPtr {
            auto ic = text(icon);
            ic->fontSize(20.0f);

            auto hd = text(heading);
            hd->fontSize(14.5f).bold().color(0xFFF1F5F9);

            std::vector<WidgetPtr> hd_items = {ic, hd};
            auto hd_row = row(hd_items);
            hd_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

            auto ds = text(desc);
            ds->fontSize(12.0f).color(0xFF94A3B8);

            auto b_lbl = text(btn_label);
            b_lbl->fontSize(12.5f).bold().color(0xFFFFFFFF);

            auto b_box = container(b_lbl);
            b_box->color(btn_col)
                 .borderRadius(6.0f)
                 .paddingSymmetric(8.0f, 16.0f);

            auto gd = std::make_shared<GestureDetector>(b_box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, mode, heading](const TapUpDetails&) {
                current_dialog_mode_ = mode;
                hud_msg_ = "Opened: " + heading;
                dialog_ctrl_->show();
                setState([] {});
            };

            std::vector<WidgetPtr> c_items = {hd_row, ds, gd};
            auto col = column(c_items);
            col->gap(StyleValue::point(12.0f));

            auto card = container(col);
            card->color(0xFF1E293B)
                .borderRadius(10.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(16.0f)
                .width(260.0f);
            return card;
        };

        auto card1 = makeCard("🗑️", "Destructive Action", "Delete database cluster with danger confirmation.",
                              "⚠️ Delete Cluster", 0xFFDC2626, "delete");

        auto card2 = makeCard("🚀", "Deployment Ready", "Review production release and Edge statistics.",
                              "✅ View Deployment", 0xFF059669, "success");

        auto card3 = makeCard("👥", "Invite Member", "Add new architect to cloud organization.",
                              "✉️ Invite Member", 0xFF0284C7, "invite");

        auto card4 = makeCard("🔒", "Activate License", "Unlock Enterprise Vulkan GPU pipeline.",
                              "🔑 Enter License", 0xFFD97706, "license");

        std::vector<WidgetPtr> cards_list = {card1, card2, card3, card4};
        auto cards_row = row(cards_list);
        cards_row->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(1082.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, cards_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

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

        auto modal_dialog = dialog(active_content, background_page, dialog_opts);
        modal_dialog->setController(dialog_ctrl_);

        return modal_dialog;
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
