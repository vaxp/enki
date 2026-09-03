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
        return container({
            .color = 0x3D000000,
            .padding = StyleInsets::all(16.0f),
            .flex_grow = 1.0f,
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(14.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .children = {
                            text("Advanced Timeline & Process Suite", {
                                .color = 0xFFFFFFFF,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold
                            }),
                            text("Horizontal CI/CD steppers, vertical alternate milestone changelogs, and real-time logistics tracking", {
                                .color = 0xFF94A3B8,
                                .font_size = 13.0f
                            })
                        }
                    }),
                    
                    container({
                        .color = 0x4D000000,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(1220.0f),
                        .padding = StyleInsets::all(16.0f),
                        .child = column({
                            .gap = StyleValue::point(10.0f),
                            .children = {
                                row({
                                    .justify_content = Justify::SpaceBetween,
                                    .align_items = Align::Center,
                                    .children = {
                                        text("1. CI/CD Deployment Pipeline (Horizontal Interactive Stepper)", {
                                            .color = 0xFF38BDF8,
                                            .font_size = 14.0f,
                                            .font_weight = FontWeight::Bold
                                        }),
                                        row({
                                            .align_items = Align::Center,
                                            .gap = StyleValue::point(8.0f),
                                            .children = {
                                                button({
                                                    .child = text("⏮ Previous Step"),
                                                    .on_pressed = [this] {
                                                        pipeline_ctrl_->prevStep();
                                                        hud_msg_ = "Active Step: #" + std::to_string(pipeline_ctrl_->getActiveStep() + 1);
                                                        setState([] {});
                                                    }
                                                }),
                                                button({
                                                    .child = text("Next Step ➔"),
                                                    .on_pressed = [this] {
                                                        pipeline_ctrl_->nextStep();
                                                        hud_msg_ = "Active Step: #" + std::to_string(pipeline_ctrl_->getActiveStep() + 1);
                                                        setState([] {});
                                                    }
                                                })
                                            }
                                        })
                                    }
                                }),
                                Timeline {
                                    .controller = pipeline_ctrl_,
                                    .orientation = TimelineOrientation::Horizontal,
                                    .node_size = 28.0f,
                                    .card_width = 190.0f,
                                    .is_stepper = true,
                                    .on_step_changed = [this](int step) {
                                        hud_msg_ = "Switched to Pipeline Step #" + std::to_string(step + 1);
                                        setState([] {});
                                    }
                                }
                            }
                        })
                    }),

                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(20.0f),
                        .children = {
                            container({
                                .color = 0x4D000000,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(600.0f),
                                .padding = StyleInsets::all(16.0f),
                                .child = column({
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("2. Release Milestones & Changelog (Vertical Alternate Zig-Zag)", {
                                            .color = 0xFF10B981,
                                            .font_size = 14.0f,
                                            .font_weight = FontWeight::Bold
                                        }),
                                        text("Click on any milestone card to expand or collapse detailed changelog notes.", {
                                            .color = 0xFF94A3B8,
                                            .font_size = 12.0f
                                        }),
                                        Timeline {
                                            .controller = milestone_ctrl_,
                                            .orientation = TimelineOrientation::Vertical,
                                            .alignment = TimelineAlignment::Alternate,
                                            .node_size = 26.0f,
                                            .card_width = 250.0f,
                                            .on_item_expanded = [this](const std::string& id, bool exp) {
                                                hud_msg_ = "Toggled milestone " + id + (exp ? " (Expanded details)" : " (Collapsed)");
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            }),
                            container({
                                .color = 0x4D000000,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(600.0f),
                                .padding = StyleInsets::all(16.0f),
                                .child = column({
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("3. Real-Time Shipment Tracking (Vertical Start Aligned)", {
                                            .color = 0xFFF59E0B,
                                            .font_size = 14.0f,
                                            .font_weight = FontWeight::Bold
                                        }),
                                        text("Live delivery status with custom transport icons and status colors.", {
                                            .color = 0xFF94A3B8,
                                            .font_size = 12.0f
                                        }),
                                        Timeline {
                                            .controller = logistics_ctrl_,
                                            .orientation = TimelineOrientation::Vertical,
                                            .alignment = TimelineAlignment::Start,
                                            .node_size = 26.0f,
                                            .item_spacing = 14.0f,
                                            .on_item_tap = [this](const TimelineItem& it) {
                                                hud_msg_ = "Selected Logistics Event: " + it.title;
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            })
                        }
                    }),

                    container({
                        .color = 0x4D000000,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF553333, 1.0f),
                        .width = StyleValue::point(1220.0f),
                        .padding = StyleInsets::symmetric(6.0f, 12.0f),
                        .child = row({
                            .children = {
                                text("💡 " + hud_msg_, {
                                    .color = 0xFF38BDF8,
                                    .font_size = 12.0f
                                })
                            }
                        })
                    })
                }
            })
        });
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
    config.target_fps  = 0;
    config.show_performance_overlay = false;
    config.clear_color = 0x2D000000;

    return runApp(std::make_shared<TimelineDemoApp>(), config);
}
