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
        auto title = text("Advanced Notification System", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Dual presentation: Floating Push Toast Banners + In-App Notification Center Feed", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto t_col = column({
            .gap = StyleValue::point(4.0f),
            .children = {title, sub}
        });

        // Bell widget with live unread badge
        auto bell = NotificationBell { .manager = notify_mgr_ };

        auto top_bar = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::point(1080.0f),
            .children = {t_col, bell}
        });

        // ── Helper to build trigger cards ─────────────────────────────
        auto makePushCard = [this](std::string icon, std::string title, std::string desc,
                                   std::string btn_label, Color btn_col, std::function<void()> cb) -> WidgetPtr {
            auto ic = text(icon, { .font_size = 20.0f });

            auto tit = text(title, {
                .color = 0xFFF1F5F9,
                .font_size = 14.5f,
                .font_weight = FontWeight::Bold,
            });

            auto h_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {ic, tit}
            });

            auto ds = text(desc, {
                .color = 0xFF94A3B8,
                .font_size = 12.0f,
            });

            auto b_lbl = text(btn_label, {
                .color = 0xFFFFFFFF,
                .font_size = 12.5f,
                .font_weight = FontWeight::Bold,
            });

            auto b_box = container({
                .color = btn_col,
                .border_radius = BorderRadius::circular(6.0f),
                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                .child = b_lbl
            });

            auto gd = gestureDetector({
                .child = b_box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [cb](const TapUpDetails&) {
                    if (cb) cb();
                },
            });

            auto col = column({
                .gap = StyleValue::point(12.0f),
                .children = {h_row, ds, gd}
            });

            return container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::point(250.0f),
                .padding = StyleInsets::all(16.0f),
                .child = col
            });
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

        // ── Feed Quick Action Bar ─────────────────────────────────────
        auto btn_open_feed = button(text("📂 Open Notification Center Drawer", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }), [this] {
            notify_mgr_->openCenter();
            hud_msg_ = "Opened In-App Notification Center Drawer.";
            setState([] {});
        });

        auto btn_read_all = button(text("✓ Mark All As Read", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }), [this] {
            notify_mgr_->markAllAsRead();
            hud_msg_ = "Marked all notifications as read.";
            setState([] {});
        });

        auto btn_clear = button(text("🗑️ Clear All Feed Items", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }), [this] {
            notify_mgr_->clearAll();
            hud_msg_ = "Cleared all notification feed items.";
            setState([] {});
        });

        // Wrap with NotificationOverlay
        return NotificationOverlay {
            .body = container({
                .color = 0xFF0B1120,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(24.0f),
                .child = column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(26.0f),
                    .children = {
                        top_bar,
                        row({
                            .justify_content = Justify::Center,
                            .gap = StyleValue::point(14.0f),
                            .children = {card1, card2, card3, card4}
                        }),
                        row({
                            .justify_content = Justify::Center,
                            .gap = StyleValue::point(12.0f),
                            .children = { btn_open_feed, btn_read_all, btn_clear }
                        }),
                        container({
                            .color = 0xFF1E293B,
                            .border_radius = BorderRadius::circular(6.0f),
                            .border = Border(0xFF334155, 1.0f),
                            .width = StyleValue::point(1080.0f),
                            .padding = StyleInsets::symmetric(8.0f, 16.0f),
                            .child = row({
                                .children = { text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }) }
                            })
                        })
                    }
                })
            }),
            .manager = notify_mgr_
        };
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
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<NotificationDemoApp>(), config);
}
