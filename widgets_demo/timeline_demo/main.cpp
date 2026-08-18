/// @file main.cpp
/// @brief ENKI Advanced Timeline Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/timeline.hpp"
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

static std::vector<TimelineItem> buildPipelineItems() {
    std::vector<TimelineItem> items;

    TimelineItem it1("s1", "Build Engine", "00:45s", "Compile libenki core C++20", TimelineItemStatus::Completed);
    it1.setIcon("📦").setBadge("PASS", 0x2E10B981, 0xFFFFFFFF);
    items.push_back(it1);

    TimelineItem it2("s2", "Unit & Tree Tests", "01:12s", "89 test suites executed cleanly", TimelineItemStatus::Completed);
    it2.setIcon("🧪").setBadge("100%", 0x2E10B981, 0xFFFFFFFF);
    items.push_back(it2);

    TimelineItem it3("s3", "Security Audit", "In progress...", "Memory safety & ABI verification", TimelineItemStatus::Active);
    it3.setIcon("🔒").setBadge("ACTIVE", 0x2E38BDF8, 0xFFFFFFFF);
    items.push_back(it3);

    TimelineItem it4("s4", "Deploy Staging", "Pending", "Deploy to Wayland QA environment", TimelineItemStatus::Pending);
    it4.setIcon("🌐").setBadge("QUEUED", 0x2E475569, 0xFFFFFFFF);
    items.push_back(it4);

    TimelineItem it5("s5", "Production Release", "Pending", "Publish v1.0.0 binaries and SDK", TimelineItemStatus::Pending);
    it5.setIcon("🚀").setBadge("v1.0.0", 0x2E475569, 0xFFFFFFFF);
    items.push_back(it5);

    return items;
}

static std::vector<TimelineItem> buildMilestoneItems() {
    std::vector<TimelineItem> items;

    TimelineItem m1("m1", "ENKI 2.0 — Skia Native Engine", "August 2026",
                   "Direct GPU hardware acceleration via Skia Canvas with custom rasterizers.",
                   TimelineItemStatus::Completed);
    m1.setBadge("RELEASE", 0x2E10B981, 0xFFFFFFFF)
      .setIcon("🚀")
      .setDetails("• Complete migration to Skia Canvas\n• Text layout via SkParagraph & SkFontMgr\n• Anti-aliased RRect clips and vector rendering\n• 60+ FPS smooth animations");
    items.push_back(m1);

    TimelineItem m2("m2", "ENKI 1.5 — Anu Flexbox Layout", "June 2026",
                   "CSS-compliant Flexbox layout engine integrated into widget element tree.",
                   TimelineItemStatus::Completed);
    m2.setBadge("CORE", 0x2E38BDF8, 0xFFFFFFFF)
      .setIcon("📐")
      .setDetails("• Flexible stretch, grow, shrink, and wrap\n• Precise pixel measurement with Yoga/Anu bindings\n• Native multi-pass layout resolution");
    items.push_back(m2);

    TimelineItem m3("m3", "ENKI 1.0 — Wayland Compositor", "April 2026",
                   "Direct Wayland client platform backend with pure shared memory & EGL.",
                   TimelineItemStatus::Completed);
    m3.setBadge("INIT", 0x2EF59E0B, 0xFFFFFFFF)
      .setIcon("⚡")
      .setDetails("• Wayland pointer, keyboard, clipboard protocols\n• High-DPI fractional surface scaling\n• Low-latency input handling");
    items.push_back(m3);

    return items;
}

static std::vector<TimelineItem> buildLogisticsItems() {
    std::vector<TimelineItem> items;

    TimelineItem it1("l1", "Order Confirmed & Payment Verified", "Today, 09:15 AM", "Transaction #TRX-94829 processed via Stripe", TimelineItemStatus::Completed);
    it1.setIcon("💳");
    items.push_back(it1);

    TimelineItem it2("l2", "Package Dispatched from Hub", "Today, 11:30 AM", "Carrier: DHL Express Tracking #DHL-883920", TimelineItemStatus::Completed);
    it2.setIcon("📦");
    items.push_back(it2);

    TimelineItem it3("l3", "Out for Delivery", "Today, 14:10 PM", "Courier is en route to destination address", TimelineItemStatus::Active);
    it3.setIcon("🚚");
    items.push_back(it3);

    return items;
}

class TimelineDemoState : public State {
private:
    std::shared_ptr<TimelineController> pipeline_ctrl_;
    std::shared_ptr<TimelineController> milestone_ctrl_;
    std::shared_ptr<TimelineController> logistics_ctrl_;
    std::string hud_msg_ = "Click on pipeline steps or milestone cards to interact!";

public:
    void initState() override {
        State::initState();
        pipeline_ctrl_ = std::make_shared<TimelineController>(buildPipelineItems(), 2);
        milestone_ctrl_ = std::make_shared<TimelineController>(buildMilestoneItems());
        logistics_ctrl_ = std::make_shared<TimelineController>(buildLogisticsItems());
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced Timeline & Process Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Horizontal CI/CD steppers, vertical alternate milestone changelogs, and real-time logistics tracking");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── 1. Horizontal CI/CD Stepper Card ──────────────────────────
        TimelineOptions pipe_opts;
        pipe_opts.orientation = TimelineOrientation::Horizontal;
        pipe_opts.is_stepper = true;
        pipe_opts.node_size = 28.0f;
        pipe_opts.card_width = 190.0f;
        pipe_opts.on_step_changed = [this](int step) {
            hud_msg_ = "Switched to Pipeline Step #" + std::to_string(step + 1);
            setState([] {});
        };

        auto pipe_widget = timeline(pipeline_ctrl_, pipe_opts);

        auto pipe_title = text("1. CI/CD Deployment Pipeline (Horizontal Interactive Stepper)");
        pipe_title->fontSize(14.0f).bold().color(0xFF38BDF8);

        auto btn_prev = button(text("⏮ Previous Step"), [this] {
            pipeline_ctrl_->prevStep();
            hud_msg_ = "Active Step: #" + std::to_string(pipeline_ctrl_->getActiveStep() + 1);
            setState([] {});
        });

        auto btn_next = button(text("Next Step ➔"), [this] {
            pipeline_ctrl_->nextStep();
            hud_msg_ = "Active Step: #" + std::to_string(pipeline_ctrl_->getActiveStep() + 1);
            setState([] {});
        });

        std::vector<WidgetPtr> pipe_btn_items = {btn_prev, btn_next};
        auto pipe_btn_row = row(pipe_btn_items);
        pipe_btn_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> pipe_hdr_items = {pipe_title, pipe_btn_row};
        auto pipe_hdr_row = row(pipe_hdr_items);
        pipe_hdr_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center);

        std::vector<WidgetPtr> c1_items = {pipe_hdr_row, pipe_widget};
        auto c1_col = column(c1_items);
        c1_col->gap(StyleValue::point(10.0f));

        auto card1 = container(c1_col);
        card1->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(1220.0f);

        // ── 2. Vertical Alternate Milestone Changelog Card ────────────
        TimelineOptions mile_opts;
        mile_opts.orientation = TimelineOrientation::Vertical;
        mile_opts.alignment = TimelineAlignment::Alternate;
        mile_opts.node_size = 26.0f;
        mile_opts.card_width = 250.0f;
        mile_opts.on_item_expanded = [this](const std::string& id, bool exp) {
            hud_msg_ = "Toggled milestone " + id + (exp ? " (Expanded details)" : " (Collapsed)");
            setState([] {});
        };

        auto mile_widget = timeline(milestone_ctrl_, mile_opts);

        auto c2_title = text("2. Release Milestones & Changelog (Vertical Alternate Zig-Zag)");
        c2_title->fontSize(14.0f).bold().color(0xFF10B981);

        auto c2_sub = text("Click on any milestone card to expand or collapse detailed changelog notes.");
        c2_sub->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c2_items = {c2_title, c2_sub, mile_widget};
        auto c2_col = column(c2_items);
        c2_col->gap(StyleValue::point(10.0f));

        auto card2 = container(c2_col);
        card2->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(600.0f);

        // ── 3. Vertical Standard Logistics Tracking Card ──────────────
        TimelineOptions log_opts;
        log_opts.orientation = TimelineOrientation::Vertical;
        log_opts.alignment = TimelineAlignment::Start;
        log_opts.node_size = 26.0f;
        log_opts.item_spacing = 14.0f;
        log_opts.on_item_tap = [this](const TimelineItem& it) {
            hud_msg_ = "Selected Logistics Event: " + it.title;
            setState([] {});
        };

        auto log_widget = timeline(logistics_ctrl_, log_opts);

        auto c3_title = text("3. Real-Time Shipment Tracking (Vertical Start Aligned)");
        c3_title->fontSize(14.0f).bold().color(0xFFF59E0B);

        auto c3_sub = text("Live delivery status with custom transport icons and status colors.");
        c3_sub->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c3_items = {c3_title, c3_sub, log_widget};
        auto c3_col = column(c3_items);
        c3_col->gap(StyleValue::point(10.0f));

        auto card3 = container(c3_col);
        card3->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(600.0f);

        // Bottom Row: Card2 & Card3
        std::vector<WidgetPtr> bot_items = {card2, card3};
        auto bot_row = row(bot_items);
        bot_row->gap(StyleValue::point(20.0f))
               .justifyContent(Justify::Center);

        // HUD / Status
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.0f).color(0xFF38BDF8);

        std::vector<WidgetPtr> hud_items = {hud_txt};
        auto hud_row = row(hud_items);
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(6.0f, 12.0f)
               .width(1220.0f);

        // Main Page Stack
        std::vector<WidgetPtr> page_items = {title_col, card1, bot_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(14.0f))
                .alignItems(Align::Center);

        auto app_root = container(page_col);
        app_root->color(0xFF0B1120)
                .paddingAll(16.0f)
                .flexGrow(1.0f);

        return app_root;
    }
};

class TimelineDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TimelineDemoState>();
    }
    std::string_view typeName() const override { return "TimelineDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Timeline Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Timeline Demo";
    config.width       = 1300;
    config.height      = 960;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<TimelineDemoApp>(), config);
}
