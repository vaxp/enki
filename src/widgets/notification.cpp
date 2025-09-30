#include "enki/widgets/notification.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// NotificationBell Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr NotificationBellWidget::build(BuildContext&) {
    auto ic_txt = text({
        .text = bell_icon,
        .font_size = 16.0f,
    });

    std::vector<WidgetPtr> bell_items = {ic_txt};

    int unread = manager ? manager->getUnreadCount() : 0;
    if (unread > 0) {
        std::string count_str = unread > 99 ? "99+" : std::to_string(unread);
        auto badge_txt = text({
            .text = count_str,
            .color = 0xFFFFFFFF,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        });

        auto badge_box = container({
            .color = 0xFFEF4444,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = StyleInsets::symmetric(1.0f, 5.0f),
            .child = badge_txt,
        });
        bell_items.push_back(badge_box);
    }

    auto bell_row = row({
        .align_items = Align::Center,
        .gap = StyleValue::point(6.0f),
        .children = std::move(bell_items),
    });

    auto box = container({
        .color = unread > 0 ? 0x22EF4444 : 0xFF1E293B,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(unread > 0 ? 0xFFEF4444 : 0xFF334155, 1.0f),
        .padding = StyleInsets::symmetric(6.0f, 12.0f),
        .child = bell_row,
    });

    auto mgr = manager;
    return gestureDetector({
        .child = box,
        .cursor_type = SystemCursor::Pointer,
        .on_tap_up = [mgr](const TapUpDetails&) {
            if (mgr) mgr->toggleCenter();
        },
    });
}

// ════════════════════════════════════════════════════════════════
// NotificationCenter Scrim
// ════════════════════════════════════════════════════════════════

class RenderNotificationScrim : public RenderBox {
public:
    std::function<void()> on_tap;

    explicit RenderNotificationScrim(std::function<void()> tap) : on_tap(std::move(tap)) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext& ctx) override {
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = size_.height;
        Paint p;
        p.setColor(0x80000000); // 50% dark backdrop
        ctx.canvas.drawRect(Rect{x, y, w, h}, p);
    }

    bool hitTestSelf(Point) const override { return true; }
    void handlePointerDown(const PointerEvent&) override {
        if (on_tap) on_tap();
    }
};

class NotificationScrimWidget : public SingleChildRenderObjectWidget {
public:
    std::function<void()> on_tap;
    explicit NotificationScrimWidget(std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none()), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderNotificationScrim>(on_tap);
    }
    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderNotificationScrim&>(ro);
        r.on_tap = on_tap;
    }
    [[nodiscard]] std::string_view typeName() const override { return "NotificationScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// NotificationOverlay State Implementation
// ════════════════════════════════════════════════════════════════

class NotificationOverlayState : public State {
private:
    NotificationCategory selected_category_ = NotificationCategory::All;
    SlotId key_down_conn_ = 0;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const NotificationOverlayWidget*>(widget());
        if (w->manager) {
            w->manager->addListener([this] {
                setState([] {});
            });
        }

        if (Platform::instance()) {
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int) {
                if (key == 0xff1b) { // Escape key
                    auto* w = static_cast<const NotificationOverlayWidget*>(widget());
                    if (w->manager && w->manager->isCenterOpen()) {
                        w->manager->closeCenter();
                    }
                }
            });
        }
    }

    void dispose() override {
        if (Platform::instance() && key_down_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    // ── Floating Push Toast Card Builder ──────────────────────────

    WidgetPtr buildPushToastCard(const NotificationItem& item, std::shared_ptr<NotificationManager> mgr) {
        Color border_col = 0xFF334155;
        Color badge_bg = 0x2E38BDF8;

        if (item.severity == NotificationSeverity::Success) {
            border_col = 0xFF10B981; badge_bg = 0x2E10B981;
        } else if (item.severity == NotificationSeverity::Error) {
            border_col = 0xFFEF4444; badge_bg = 0x2EEF4444;
        } else if (item.severity == NotificationSeverity::Warning) {
            border_col = 0xFFF59E0B; badge_bg = 0x2EF59E0B;
        } else if (item.severity == NotificationSeverity::Security) {
            border_col = 0xFF8B5CF6; badge_bg = 0x2E8B5CF6;
        }

        // Icon Badge
        auto ic_txt = text({
            .text = item.icon,
            .font_size = 16.0f,
        });
        auto ic_box = container({
            .color = badge_bg,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::all(6.0f),
            .child = ic_txt,
        });

        // Title + Timestamp
        auto title_txt = text({
            .text = item.title,
            .color = 0xFFFFFFFF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });

        auto time_txt = text({
            .text = item.timestamp_str,
            .color = 0xFF94A3B8,
            .font_size = 11.0f,
        });

        auto top_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {title_txt, time_txt},
        });

        // Message text
        auto msg_txt = text({
            .text = item.message,
            .color = 0xFFCBD5E1,
            .font_size = 12.0f,
        });

        std::vector<WidgetPtr> text_col_items = {top_row, msg_txt};

        // Action Buttons Row (if present)
        if (!item.actions.empty()) {
            std::vector<WidgetPtr> act_btns;
            for (const auto& act : item.actions) {
                Color bg = act.is_danger ? 0xFFDC2626 : (act.is_primary ? 0xFF0284C7 : 0xFF0F172A);
                Color border = act.is_danger ? 0xFFEF4444 : (act.is_primary ? 0xFF38BDF8 : 0xFF334155);

                auto b_txt = text({
                    .text = act.label,
                    .color = 0xFFFFFFFF,
                    .font_size = 11.5f,
                    .font_weight = FontWeight::Bold,
                });

                auto b_box = container({
                    .color = bg,
                    .border_radius = BorderRadius::circular(4.0f),
                    .border = Border(border, 1.0f),
                    .padding = StyleInsets::symmetric(4.0f, 10.0f),
                    .child = b_txt,
                });

                auto act_copy = act;
                auto id_copy = item.id;
                auto gd = gestureDetector({
                    .child = b_box,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap_up = [mgr, act_copy, id_copy](const TapUpDetails&) {
                        if (act_copy.on_click) act_copy.on_click();
                        if (mgr) mgr->dismissPushToast(id_copy);
                    },
                });
                act_btns.push_back(gd);
            }
            auto act_row = row({
                .gap = StyleValue::point(6.0f),
                .children = std::move(act_btns),
            });
            text_col_items.push_back(act_row);
        }

        auto text_col = column({
            .flex = 1.0f,
            .gap = StyleValue::point(4.0f),
            .children = std::move(text_col_items),
        });

        // Close button ✕
        auto cls_txt = text({
            .text = "✕",
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
            .font_weight = FontWeight::Bold,
        });
        auto cls_box = container({
            .padding = StyleInsets::all(4.0f),
            .child = cls_txt,
        });
        auto item_id = item.id;
        auto cls_btn = gestureDetector({
            .child = cls_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr, item_id](const TapUpDetails&) {
                if (mgr) mgr->dismissPushToast(item_id);
            },
        });

        auto main_row = row({
            .align_items = Align::Start,
            .gap = StyleValue::point(10.0f),
            .children = {ic_box, text_col, cls_btn},
        });

        auto card_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(border_col, 1.0f),
            .box_shadow = {BoxShadow(0x99000000, {0.0f, 8.0f}, 20.0f)},
            .width = StyleValue::point(380.0f),
            .padding = StyleInsets::all(12.0f),
            .child = main_row,
        });

        return card_box;
    }

    // ── In-App Notification Center Drawer Feed Builder ────────────

    WidgetPtr buildNotificationCenter(std::shared_ptr<NotificationManager> mgr) {
        // 1. Center Header
        auto title_lbl = text({
            .text = "Notification Feed",
            .color = 0xFFFFFFFF,
            .font_size = 16.0f,
            .font_weight = FontWeight::Bold,
        });

        int unread = mgr ? mgr->getUnreadCount() : 0;
        auto badge_txt = text({
            .text = std::to_string(unread) + " unread",
            .color = 0xFF38BDF8,
            .font_size = 11.0f,
        });
        auto badge_box = container({
            .color = 0x2E38BDF8,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(2.0f, 6.0f),
            .child = badge_txt,
        });

        auto t_left_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {title_lbl, badge_box},
        });

        // Header Action: Mark all read
        auto mark_txt = text({
            .text = "✓ Read All",
            .color = 0xFF94A3B8,
            .font_size = 11.5f,
        });
        auto mark_box = container({
            .padding = StyleInsets::all(4.0f),
            .child = mark_txt,
        });
        auto mark_btn = gestureDetector({
            .child = mark_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr](const TapUpDetails&) {
                if (mgr) mgr->markAllAsRead();
            },
        });

        // Header Action: Clear all
        auto clr_txt = text({
            .text = "🗑️ Clear",
            .color = 0xFFEF4444,
            .font_size = 11.5f,
        });
        auto clr_box = container({
            .padding = StyleInsets::all(4.0f),
            .child = clr_txt,
        });
        auto clr_btn = gestureDetector({
            .child = clr_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr](const TapUpDetails&) {
                if (mgr) mgr->clearAll();
            },
        });

        // Close ✕
        auto cls_txt = text({
            .text = "✕",
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });
        auto cls_box = container({
            .padding = StyleInsets::all(4.0f),
            .child = cls_txt,
        });
        auto cls_btn = gestureDetector({
            .child = cls_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr](const TapUpDetails&) {
                if (mgr) mgr->closeCenter();
            },
        });

        auto h_right_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {mark_btn, clr_btn, cls_btn},
        });

        auto h_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {t_left_row, h_right_row},
        });

        // 2. Category Filter Tabs
        auto makeTab = [this](std::string label, NotificationCategory cat) -> WidgetPtr {
            bool is_act = (selected_category_ == cat);
            auto t = text({
                .text = label,
                .color = is_act ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 11.5f,
                .font_weight = is_act ? FontWeight::Bold : FontWeight::Normal,
            });

            auto b = container({
                .color = is_act ? 0xFF0284C7 : 0xFF0F172A,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(is_act ? 0xFF38BDF8 : 0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(4.0f, 10.0f),
                .child = t,
            });

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, cat](const TapUpDetails&) {
                    selected_category_ = cat;
                    setState([] {});
                },
            });
        };

        auto tabs_row = row({
            .gap = StyleValue::point(6.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                makeTab("All", NotificationCategory::All),
                makeTab("Security", NotificationCategory::Security),
                makeTab("System", NotificationCategory::System),
                makeTab("Mentions", NotificationCategory::Mentions),
                makeTab("Updates", NotificationCategory::Updates)
            },
        });

        // 3. Notification Items List
        std::vector<WidgetPtr> list_rows;
        if (mgr) {
            for (const auto& item : mgr->getAllItems()) {
                if (selected_category_ != NotificationCategory::All && item.category != selected_category_) {
                    continue;
                }

                // Unread dot indicator
                std::vector<WidgetPtr> row_elements;
                if (!item.is_read) {
                    auto dot = container({
                        .color = 0xFF38BDF8,
                        .border_radius = BorderRadius::circular(4.0f),
                        .width = StyleValue::point(6.0f),
                        .height = StyleValue::point(6.0f),
                    });
                    row_elements.push_back(dot);
                }

                // Severity icon
                auto ic = text({
                    .text = item.icon,
                    .font_size = 14.0f,
                });
                row_elements.push_back(ic);

                // Title + Subtitle
                auto tit = text({
                    .text = item.title,
                    .color = item.is_read ? 0xFF94A3B8 : 0xFFFFFFFF,
                    .font_size = 12.5f,
                    .font_weight = !item.is_read ? FontWeight::Bold : FontWeight::Normal,
                });

                auto msg = text({
                    .text = item.message,
                    .color = 0xFF64748B,
                    .font_size = 11.5f,
                });

                auto tm = text({
                    .text = item.timestamp_str,
                    .color = 0xFF64748B,
                    .font_size = 10.5f,
                });

                auto col_txt = column({
                    .flex = 1.0f,
                    .gap = StyleValue::point(2.0f),
                    .children = {tit, msg, tm},
                });
                row_elements.push_back(col_txt);

                auto item_row = row({
                    .align_items = Align::Start,
                    .gap = StyleValue::point(8.0f),
                    .width = StyleValue::percent(100.0f),
                    .children = std::move(row_elements),
                });

                auto item_box = container({
                    .color = item.is_read ? 0xFF0F172A : 0x221E293B,
                    .border_radius = BorderRadius::circular(8.0f),
                    .border = Border(item.is_read ? 0xFF1E293B : 0xFF334155, 1.0f),
                    .width = StyleValue::percent(100.0f),
                    .padding = StyleInsets::all(10.0f),
                    .child = item_row,
                });

                auto id_copy = item.id;
                auto gd = gestureDetector({
                    .child = item_box,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap_up = [mgr, id_copy](const TapUpDetails&) {
                        if (mgr) mgr->markAsRead(id_copy);
                    },
                });
                list_rows.push_back(gd);
            }
        }

        if (list_rows.empty()) {
            auto empty_txt = text({
                .text = "🎉 All caught up! No notifications.",
                .color = 0xFF94A3B8,
                .font_size = 13.0f,
            });
            auto empty_box = container({
                .padding = StyleInsets::all(30.0f),
                .child = empty_txt,
            });
            list_rows.push_back(empty_box);
        }

        auto list_col = column({
            .gap = StyleValue::point(8.0f),
            .width = StyleValue::percent(100.0f),
            .children = std::move(list_rows),
        });

        auto panel_col = column({
            .gap = StyleValue::point(14.0f),
            .width = StyleValue::percent(100.0f),
            .children = {h_row, tabs_row, list_col},
        });

        auto panel_box = container({
            .color = 0xFF1E293B,
            .border = Border(0xFF334155, 1.0f),
            .box_shadow = {BoxShadow(0x99000000, {-8.0f, 0.0f}, 24.0f)},
            .width = StyleValue::point(380.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = panel_col,
        });

        return panel_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const NotificationOverlayWidget*>(widget());

        // ── 1. Invariant Page Body ────────────────────────────────────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .child = w->body,
            });
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
            });
            body_widget = Positioned::fill(empty);
        }

        std::vector<WidgetPtr> stack_items = {body_widget};

        // ── 2. Floating Push Toast Banners (Top-Right) ────────────────
        if (w->manager && !w->manager->getActivePushToasts().empty()) {
            std::vector<WidgetPtr> toast_cards;
            for (const auto& toast : w->manager->getActivePushToasts()) {
                toast_cards.push_back(buildPushToastCard(toast, w->manager));
            }
            auto toasts_col = column({
                .gap = StyleValue::point(10.0f),
                .children = std::move(toast_cards),
            });

            stack_items.push_back(Positioned {
                .child = toasts_col,
                .top = StyleValue::point(20.0f),
                .right = StyleValue::point(20.0f),
            });
        }

        // ── 3. In-App Notification Center Drawer ──────────────────────
        if (w->manager && w->manager->isCenterOpen()) {
            auto mgr = w->manager;
            auto scrim = std::make_shared<NotificationScrimWidget>([mgr] {
                if (mgr) mgr->closeCenter();
            });
            stack_items.push_back(scrim);

            auto center_panel = buildNotificationCenter(w->manager);
            stack_items.push_back(Positioned {
                .child = center_panel,
                .top = StyleValue::point(0.0f),
                .right = StyleValue::point(0.0f),
                .bottom = StyleValue::point(0.0f),
                .width = StyleValue::point(380.0f),
            });
        }

        return Stack {
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = std::move(stack_items),
        };
    }
};

std::unique_ptr<State> NotificationOverlayWidget::createState() {
    return std::make_unique<NotificationOverlayState>();
}

} // namespace enki
