/// @file main.cpp
/// @brief ENKI Advanced Notification System Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/notification.hpp"
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

class NotificationDemoState : public State {
private:
    std::shared_ptr<NotificationManager> notify_mgr_;
    std::string hud_msg_ = "Click any card button to fire interactive notifications, or click the Bell 🔔 to open the Feed.";
    int notif_counter_ = 0;

public:
    void initState() override {
        State::initState();
        notify_mgr_ = std::make_shared<NotificationManager>();

        // Pre-populate with initial notifications in feed
        auto n1 = NotificationItem::security("init_sec", "New Login from Tokyo, Japan", "IP 192.0.2.48 logged into organization 'Hyperion'.")
            .setTime("5m ago")
            .addAction(NotificationAction("sec_block", "Block IP", false, true, [this] {
                hud_msg_ = "Action Executed: Blocked suspicious IP address.";
                setState([] {});
            }))
            .addAction(NotificationAction("sec_allow", "Approve", true, false, [this] {
                hud_msg_ = "Action Executed: Approved Tokyo login session.";
                setState([] {});
            }));

        auto n2 = NotificationItem::success("init_k8s", "Pipeline #4928 Succeeded", "Production build deployed to 240 Edge Kubernetes nodes.")
            .setTime("12m ago")
            .addAction(NotificationAction("k8s_view", "View Logs", true, false, [this] {
                hud_msg_ = "Action Executed: Opened Kubernetes deployment logs.";
                setState([] {});
            }));

        auto n3 = NotificationItem::info("init_mention", "@alex Mentioned You", "PR #84: 'Optimized Skia multi-layered overlay compositor'.", NotificationCategory::Mentions)
            .setTime("24m ago")
            .setIcon("💬");

        notify_mgr_->post(n3, false); // in feed only
        notify_mgr_->post(n2, false); // in feed only
        notify_mgr_->post(n1, false); // in feed only
    }

    WidgetPtr build(BuildContext&) override {
        // ── Navigation Bar with Title and NotificationBell ────────────
        auto title = text("Advanced Notification System");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Dual presentation: Floating Push Toast Banners + In-App Notification Center Feed");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_items = {title, sub};
        auto t_col = column(t_items);
        t_col->gap(StyleValue::point(4.0f));

        // Bell widget with live unread badge
        auto bell = notificationBell(notify_mgr_);

        std::vector<WidgetPtr> top_bar_items = {t_col, bell};
        auto top_bar = row(top_bar_items);
        top_bar->justifyContent(Justify::SpaceBetween)
               .alignItems(Align::Center)
               .width(1080.0f);

        // ── Helper to build trigger cards ─────────────────────────────
        auto makePushCard = [this](std::string icon, std::string title, std::string desc,
                                   std::string btn_label, Color btn_col, std::function<void()> cb) -> WidgetPtr {
            auto ic = text(icon);
            ic->fontSize(20.0f);

            auto tit = text(title);
            tit->fontSize(14.5f).bold().color(0xFFF1F5F9);

            std::vector<WidgetPtr> h_items = {ic, tit};
            auto h_row = row(h_items);
            h_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

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
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };

            std::vector<WidgetPtr> c_items = {h_row, ds, gd};
            auto col = column(c_items);
            col->gap(StyleValue::point(12.0f));

            auto card = container(col);
            card->color(0xFF1E293B)
                .borderRadius(10.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(16.0f)
                .width(250.0f);
            return card;
        };

        // ── 4 Interactive Trigger Cards ───────────────────────────────
        auto card1 = makePushCard("🛡️", "Security Alert", "Simulate suspicious account access from new device.",
                                  "🔥 Fire Security Push", 0xFF7C3AED, [this] {
            notif_counter_++;
            auto item = NotificationItem::security("sec_" + std::to_string(notif_counter_),
                                                   "New Login: Tokyo, JP",
                                                   "SSH key authentication from AWS region ap-northeast-1.")
                .setTime("Just now")
                .addAction(NotificationAction("act_block", "Block IP", false, true, [this] {
                    hud_msg_ = "Action Executed: Blocked suspicious IP address.";
                    setState([] {});
                }))
                .addAction(NotificationAction("act_ok", "Approve", true, false, [this] {
                    hud_msg_ = "Action Executed: Approved Tokyo session.";
                    setState([] {});
                }));
            notify_mgr_->post(item);
            hud_msg_ = "Fired: Security Push Toast with interactive Block/Approve actions.";
            setState([] {});
        });

        auto card2 = makePushCard("🚀", "Pipeline Success", "Simulate successful automated CI/CD deployment.",
                                  "✅ Fire Deploy Push", 0xFF059669, [this] {
            notif_counter_++;
            auto item = NotificationItem::success("deploy_" + std::to_string(notif_counter_),
                                                  "Build #493" + std::to_string(notif_counter_) + " Deployed",
                                                  "Production release is now live on Kubernetes cluster.")
                .setTime("Just now")
                .addAction(NotificationAction("act_logs", "View Logs", true, false, [this] {
                    hud_msg_ = "Action Executed: Opened live container logs.";
                    setState([] {});
                }));
            notify_mgr_->post(item);
            hud_msg_ = "Fired: Deployment Success Push Toast.";
            setState([] {});
        });

        auto card3 = makePushCard("💬", "Team Mention", "Simulate code review discussion mention.",
                                  "✉️ Fire Mention Push", 0xFF0284C7, [this] {
            notif_counter_++;
            auto item = NotificationItem::info("men_" + std::to_string(notif_counter_),
                                               "@alex Mentioned You",
                                               "PR #84: 'Can you benchmark the Skia composite tree?'",
                                               NotificationCategory::Mentions)
                .setTime("Just now")
                .setIcon("💬")
                .addAction(NotificationAction("act_reply", "Reply", true, false, [this] {
                    hud_msg_ = "Action Executed: Opened PR #84 reply editor.";
                    setState([] {});
                }));
            notify_mgr_->post(item);
            hud_msg_ = "Fired: Team Mention Push Toast.";
            setState([] {});
        });

        auto card4 = makePushCard("⚠️", "System Warning", "Simulate infrastructure disk threshold warning.",
                                  "⚠️ Fire Warning Push", 0xFFD97706, [this] {
            notif_counter_++;
            auto item = NotificationItem::warning("warn_" + std::to_string(notif_counter_),
                                                  "Database Disk 89% Full",
                                                  "Primary replica storage reached threshold quota limit.")
                .setTime("Just now")
                .addAction(NotificationAction("act_expand", "Expand Volume", true, false, [this] {
                    hud_msg_ = "Action Executed: Expanded database disk volume +50GB.";
                    setState([] {});
                }));
            notify_mgr_->post(item);
            hud_msg_ = "Fired: System Resource Warning Push Toast.";
            setState([] {});
        });

        std::vector<WidgetPtr> cards_list = {card1, card2, card3, card4};
        auto cards_row = row(cards_list);
        cards_row->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        // ── Feed Quick Action Bar ─────────────────────────────────────
        auto btn_open_feed = button(text("📂 Open Notification Center Drawer"), [this] {
            notify_mgr_->openCenter();
            hud_msg_ = "Opened In-App Notification Center Drawer.";
            setState([] {});
        });

        auto btn_read_all = button(text("✓ Mark All As Read"), [this] {
            notify_mgr_->markAllAsRead();
            hud_msg_ = "Marked all notifications as read.";
            setState([] {});
        });

        auto btn_clear = button(text("🗑️ Clear All Feed Items"), [this] {
            notify_mgr_->clearAll();
            hud_msg_ = "Cleared all notification feed items.";
            setState([] {});
        });

        std::vector<WidgetPtr> feed_actions = {btn_open_feed, btn_read_all, btn_clear};
        auto feed_row = row(feed_actions);
        feed_row->gap(StyleValue::point(12.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(1080.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {top_bar, cards_row, feed_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(26.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // Wrap with NotificationOverlay
        auto overlay = notificationOverlay(background_page, notify_mgr_);
        return overlay;
    }
};

class NotificationDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<NotificationDemoState>();
    }
    std::string_view typeName() const override { return "NotificationDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Notification System Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Notification System Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<NotificationDemoApp>(), config);
}
