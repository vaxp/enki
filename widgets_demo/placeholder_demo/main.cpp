/// @file main.cpp
/// @brief ENKI Advanced Placeholder & Skeleton Shimmer Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/placeholder.hpp"
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

class PlaceholderDemoState : public State {
private:
    std::string hud_msg_ = "Notice the live dimension badges in Blueprint mode, smooth 60fps Shimmer wave in Skeleton mode, and interactive Media Slots.";
    bool is_loading_mode_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Placeholder & Skeleton Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise scaffolding (Category 2. Basic UI), Blueprint wireframes with dimension badges, 60fps Shimmer loaders, and Media Slots");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Left Column: Blueprint Wireframe Scaffolding ──────────────
        auto bp_title = text("📐 Blueprint Wireframes & Layout Slots");
        bp_title->fontSize(15.0f).bold().color(0xFF38BDF8);

        auto slot_hero = placeholderBlueprint(320.0f, 100.0f, "Hero Banner");
        auto slot_chart = placeholderBlueprint(320.0f, 130.0f, "Analytics Chart Area");

        auto media_slot = placeholderMediaSlot("Drop Avatar or Image File Here", "🖼️", 320.0f, 90.0f);

        std::vector<WidgetPtr> bp_items = {bp_title, slot_hero, slot_chart, media_slot};
        auto bp_col = column(bp_items);
        bp_col->gap(StyleValue::point(12.0f));

        auto bp_card = container(bp_col);
        bp_card->color(0xFF0F172A)
               .border(0xFF334155, 1.0f)
               .borderRadius(12.0f)
               .paddingAll(20.0f)
               .width(360.0f);

        // ── Right Column: Skeleton Shimmer Loading Screens ────────────
        auto sk_title = text("⚡ Animated Shimmer Skeletons");
        sk_title->fontSize(15.0f).bold().color(0xFF10B981);

        // Toggle Loading Button
        auto t_btn = text(is_loading_mode_ ? "Switch to: Loaded UI" : "Switch to: Skeleton");
        t_btn->fontSize(11.5f).bold().color(0xFFFFFFFF);
        auto b_btn = container(t_btn);
        b_btn->color(is_loading_mode_ ? 0xFF0284C7 : 0xFF059669)
             .borderRadius(6.0f)
             .paddingSymmetric(6.0f, 14.0f);

        auto gd_btn = std::make_shared<GestureDetector>(b_btn);
        gd_btn->cursor_type = SystemCursor::Pointer;
        gd_btn->on_tap_up = [this](const TapUpDetails&) {
            is_loading_mode_ = !is_loading_mode_;
            hud_msg_ = is_loading_mode_ ? "Showing animated skeleton loading state" : "Showing simulated loaded content";
            setState([] {});
        };

        std::vector<WidgetPtr> head_r = {sk_title, gd_btn};
        auto head_row = row(head_r);
        head_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

        WidgetPtr display_content;

        if (is_loading_mode_) {
            // Skeleton Card & List
            auto sk_card = placeholderCardSkeleton(380.0f);
            auto sk_list = placeholderListSkeleton(3, 380.0f);

            std::vector<WidgetPtr> sk_items = {head_row, sk_card, sk_list};
            auto sk_col = column(sk_items);
            sk_col->gap(StyleValue::point(14.0f));
            display_content = sk_col;
        } else {
            // Actual Loaded Content Card
            auto t1 = text("Alex Morgan (Lead Engineer)");
            t1->fontSize(14.0f).bold().color(0xFFFFFFFF);
            auto t2 = text("Senior Distributed Systems Specialist • Tokyo, Japan");
            t2->fontSize(11.5f).color(0xFF38BDF8);
            auto t3 = text("Successfully orchestrated high-throughput rendering pipelines with 600+ FPS benchmark throughput on Linux Wayland/X11.");
            t3->fontSize(12.0f).color(0xFFCBD5E1);

            std::vector<WidgetPtr> loaded_card_items = {t1, t2, t3};
            auto lc_col = column(loaded_card_items);
            lc_col->gap(StyleValue::point(8.0f));

            auto lc_box = container(lc_col);
            lc_box->color(0xFF1E293B).border(0xFF0284C7, 1.0f).borderRadius(12.0f).paddingAll(16.0f).width(380.0f);

            // List of items
            auto makeItem = [](std::string icon, std::string title, std::string badge) -> WidgetPtr {
                auto ic = text(icon);
                ic->fontSize(14.0f);
                auto tt = text(title);
                tt->fontSize(12.5f).bold().color(0xFFFFFFFF);
                auto bg = text(badge);
                bg->fontSize(10.5f).bold().color(0xFF38BDF8);

                auto bg_box = container(bg);
                bg_box->color(0x330284C7).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

                std::vector<WidgetPtr> it_items = {ic, tt, bg_box};
                auto it_row = row(it_items);
                it_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

                auto it_box = container(it_row);
                it_box->color(0xFF1E293B).borderRadius(8.0f).paddingSymmetric(10.0f, 12.0f);
                return it_box;
            };

            auto item1 = makeItem("📊", "Performance Pipeline", "ACTIVE");
            auto item2 = makeItem("🚀", "Wayland Buffer Sync", "SYNCD");
            auto item3 = makeItem("🎨", "Skia Direct Shaders", "OPTIMAL");

            std::vector<WidgetPtr> list_items = {item1, item2, item3};
            auto list_col = column(list_items);
            list_col->gap(StyleValue::point(8.0f)).width(380.0f);

            std::vector<WidgetPtr> loaded_page_items = {head_row, lc_box, list_col};
            auto lp_col = column(loaded_page_items);
            lp_col->gap(StyleValue::point(14.0f));
            display_content = lp_col;
        }

        auto right_card = container(display_content);
        right_card->color(0xFF0F172A)
                  .border(0xFF334155, 1.0f)
                  .borderRadius(12.0f)
                  .paddingAll(20.0f)
                  .width(420.0f);

        // ── Side-by-Side Main Sections ────────────────────────────────
        std::vector<WidgetPtr> sections = {bp_card, right_card};
        auto sections_row = row(sections);
        sections_row->gap(StyleValue::point(24.0f))
                    .justifyContent(Justify::Center)
                    .alignItems(Align::Start);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(804.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, sections_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(20.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class PlaceholderDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PlaceholderDemoState>();
    }
    std::string_view typeName() const override { return "PlaceholderDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Placeholder Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Placeholder Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<PlaceholderDemoApp>(), config);
}
