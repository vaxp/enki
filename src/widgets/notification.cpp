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

        auto badge_box = container(badge_txt);
        badge_box->color(0xFFEF4444)
                 .borderRadius(10.0f)
                 .paddingSymmetric(1.0f, 5.0f);
        bell_items.push_back(badge_box);
    }

    auto bell_row = row(bell_items);
    bell_row->gap(StyleValue::point(6.0f)).alignItems(Align::Center);

    auto box = container(bell_row);
    box->color(unread > 0 ? 0x22EF4444 : 0xFF1E293B)
       .border(unread > 0 ? 0xFFEF4444 : 0xFF334155, 1.0f)
       .borderRadius(8.0f)
       .paddingSymmetric(6.0f, 12.0f);

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
        auto ic_box = container(ic_txt);
        ic_box->color(badge_bg).borderRadius(6.0f).paddingAll(6.0f);

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

        std::vector<WidgetPtr> top_row_items = {title_txt, time_txt};
        auto top_row = row(top_row_items);
        top_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center);

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

                auto b_box = container(b_txt);
                b_box->color(bg).border(border, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 10.0f);

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
            auto act_row = row(act_btns);
            act_row->gap(StyleValue::point(6.0f));
            text_col_items.push_back(act_row);
        }

        auto text_col = column(text_col_items);
        text_col->gap(StyleValue::point(4.0f)).flex(1.0f);

        // Close button ✕
        auto cls_txt = text({
            .text = "✕",
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
            .font_weight = FontWeight::Bold,
        });
        auto cls_box = container(cls_txt);
        cls_box->paddingAll(4.0f);
        auto item_id = item.id;
        auto cls_btn = gestureDetector({
            .child = cls_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr, item_id](const TapUpDetails&) {
                if (mgr) mgr->dismissPushToast(item_id);
            },
        });

        std::vector<WidgetPtr> main_row_items = {ic_box, text_col, cls_btn};
        auto main_row = row(main_row_items);
        main_row->gap(StyleValue::point(10.0f)).alignItems(Align::Start);

        auto card_box = container(main_row);
        card_box->color(0xFF1E293B)
                .border(border_col, 1.0f)
                .borderRadius(10.0f)
                .paddingAll(12.0f)
                .width(380.0f)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 20.0f));

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
        auto badge_box = container(badge_txt);
        badge_box->color(0x2E38BDF8).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

        std::vector<WidgetPtr> t_left = {title_lbl, badge_box};
        auto t_left_row = row(t_left);
        t_left_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        // Header Action: Mark all read
        auto mark_txt = text({
            .text = "✓ Read All",
            .color = 0xFF94A3B8,
            .font_size = 11.5f,
        });
        auto mark_box = container(mark_txt);
        mark_box->paddingAll(4.0f);
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
        auto clr_box = container(clr_txt);
        clr_box->paddingAll(4.0f);
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
        auto cls_box = container(cls_txt);
        cls_box->paddingAll(4.0f);
        auto cls_btn = gestureDetector({
            .child = cls_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [mgr](const TapUpDetails&) {
                if (mgr) mgr->closeCenter();
            },
        });

        std::vector<WidgetPtr> h_right = {mark_btn, clr_btn, cls_btn};
        auto h_right_row = row(h_right);
        h_right_row->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> h_items = {t_left_row, h_right_row};
        auto h_row = row(h_items);
        h_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

        // 2. Category Filter Tabs
        auto makeTab = [this](std::string label, NotificationCategory cat) -> WidgetPtr {
            bool is_act = (selected_category_ == cat);
            auto t = text({
                .text = label,
                .color = is_act ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 11.5f,
                .font_weight = is_act ? FontWeight::Bold : FontWeight::Normal,
            });

            auto b = container(t);
            b->color(is_act ? 0xFF0284C7 : 0xFF0F172A)
             .border(is_act ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(4.0f, 10.0f);

            return gestureDetector({
                .child = b,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, cat](const TapUpDetails&) {
                    selected_category_ = cat;
                    setState([] {});
                },
            });
        };

        std::vector<WidgetPtr> tab_items = {
            makeTab("All", NotificationCategory::All),
            makeTab("Security", NotificationCategory::Security),
            makeTab("System", NotificationCategory::System),
            makeTab("Mentions", NotificationCategory::Mentions),
            makeTab("Updates", NotificationCategory::Updates)
        };
        auto tabs_row = row(tab_items);
        tabs_row->gap(StyleValue::point(6.0f)).width(StyleValue::percent(100.0f));

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
                    auto dot = container();
                    dot->color(0xFF38BDF8).borderRadius(4.0f).width(6.0f).height(6.0f);
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

                std::vector<WidgetPtr> col_txt_items = {tit, msg, tm};
                auto col_txt = column(col_txt_items);
                col_txt->gap(StyleValue::point(2.0f)).flex(1.0f);
                row_elements.push_back(col_txt);

                auto item_row = row(row_elements);
                item_row->gap(StyleValue::point(8.0f)).alignItems(Align::Start).width(StyleValue::percent(100.0f));

                auto item_box = container(item_row);
                item_box->color(item.is_read ? 0xFF0F172A : 0x221E293B)
                        .border(item.is_read ? 0xFF1E293B : 0xFF334155, 1.0f)
                        .borderRadius(8.0f)
                        .paddingAll(10.0f)
                        .width(StyleValue::percent(100.0f));

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
            auto empty_box = container(empty_txt);
            empty_box->paddingAll(30.0f);
            list_rows.push_back(empty_box);
        }

        auto list_col = column(list_rows);
        list_col->gap(StyleValue::point(8.0f)).width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> panel_items = {h_row, tabs_row, list_col};
        auto panel_col = column(panel_items);
        panel_col->gap(StyleValue::point(14.0f)).width(StyleValue::percent(100.0f));

        auto panel_box = container(panel_col);
        panel_box->color(0xFF1E293B)
                 .border(0xFF334155, 1.0f)
                 .paddingAll(16.0f)
                 .width(380.0f)
                 .height(StyleValue::percent(100.0f))
                 .shadow(BoxShadow(0x99000000, {-8.0f, 0.0f}, 24.0f));

        return panel_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const NotificationOverlayWidget*>(widget());

        // ── 1. Invariant Page Body ────────────────────────────────────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container(w->body);
            bx->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(empty);
        }

        std::vector<WidgetPtr> stack_items = {body_widget};

        // ── 2. Floating Push Toast Banners (Top-Right) ────────────────
        if (w->manager && !w->manager->getActivePushToasts().empty()) {
            std::vector<WidgetPtr> toast_cards;
            for (const auto& toast : w->manager->getActivePushToasts()) {
                toast_cards.push_back(buildPushToastCard(toast, w->manager));
            }
            auto toasts_col = column(toast_cards);
            toasts_col->gap(StyleValue::point(10.0f));

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
