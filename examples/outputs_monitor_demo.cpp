/// @file outputs_monitor_demo.cpp
/// @brief ENKI Output & Multi-Monitor Management Showcase.
/// Displays all connected monitors with live hotplug detection, mode listing,
/// scale info, EDID, physical size, and rotation.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/output.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design Tokens
// ════════════════════════════════════════════════════════════════

namespace Theme {
    constexpr uint32_t bg_base        = 0xFF080C14;
    constexpr uint32_t bg_card        = 0xFF141B2D;
    constexpr uint32_t bg_badge       = 0xFF0D1525;
    constexpr uint32_t primary        = 0xFF38BDF8;
    constexpr uint32_t accent_green   = 0xFF10B981;
    constexpr uint32_t accent_amber   = 0xFFF59E0B;
    constexpr uint32_t accent_purple  = 0xFFA855F7;
    constexpr uint32_t accent_cyan    = 0xFF06B6D4;
    constexpr uint32_t text_primary   = 0xFFF1F5F9;
    constexpr uint32_t text_secondary = 0xFF94A3B8;
    constexpr uint32_t text_muted     = 0xFF475569;
    constexpr uint32_t border_subtle  = 0x18FFFFFF;
    constexpr uint32_t border_active  = 0x5038BDF8;
}

// ── Helpers ──────────────────────────────────────────────────────

static std::string fmtRefresh(int32_t mHz) {
    double hz = mHz / 1000.0;
    std::ostringstream oss;
    if (mHz % 1000 == 0)
        oss << static_cast<int>(hz) << " Hz";
    else
        oss << std::fixed << std::setprecision(2) << hz << " Hz";
    return oss.str();
}

static std::string fmtScale(int32_t scale, double frac) {
    if (frac == static_cast<double>(scale))
        return std::to_string(scale) + "\u00d7";
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << frac << "\u00d7";
    return oss.str();
}

static std::string fmtTransform(OutputTransform t) {
    switch (t) {
        case OutputTransform::Normal:     return "Normal";
        case OutputTransform::Rotated90:  return "90\u00b0";
        case OutputTransform::Rotated180: return "180\u00b0";
        case OutputTransform::Rotated270: return "270\u00b0";
        case OutputTransform::Flipped:    return "Flipped";
        case OutputTransform::Flipped90:  return "Flipped 90\u00b0";
        case OutputTransform::Flipped180: return "Flipped 180\u00b0";
        case OutputTransform::Flipped270: return "Flipped 270\u00b0";
    }
    return "Unknown";
}

// ════════════════════════════════════════════════════════════════
// App State
// ════════════════════════════════════════════════════════════════

class MonitorDemoState : public State {
public:
    std::vector<std::shared_ptr<Output>> outputs;
    int selected_idx = 0;
    std::string log_msg = "Listening for output events\u2026";

    // ── UI Builders ───────────────────────────────────────────────
    WidgetPtr buildCard(const std::shared_ptr<Output>& out, int idx);
    WidgetPtr buildModePanel(const std::shared_ptr<Output>& out);
    WidgetPtr build(BuildContext&) override;

    void initState() override {
        State::initState();

        auto* platform = Platform::instance();
        if (!platform) return;

        outputs = platform->getOutputs();
        if (!outputs.empty())
            log_msg = "Found " + std::to_string(outputs.size()) + " output(s).";

        platform->onOutputAdded().connect([this](std::shared_ptr<Output> out) {
            if (!mounted()) return;
            setState([this, out]() {
                auto* p = Platform::instance();
                if (p) outputs = p->getOutputs();
                log_msg = "\u2795  Connected: " + out->description();
            });
        });

        platform->onOutputRemoved().connect([this](std::shared_ptr<Output> out) {
            if (!mounted()) return;
            setState([this, out]() {
                auto* p = Platform::instance();
                if (p) outputs = p->getOutputs();
                if (selected_idx >= static_cast<int>(outputs.size()))
                    selected_idx = outputs.empty() ? 0 : static_cast<int>(outputs.size()) - 1;
                log_msg = "\u2796  Disconnected: " + out->name();
            });
        });

        platform->onOutputChanged().connect([this](std::shared_ptr<Output> out) {
            if (!mounted()) return;
            setState([this, out]() {
                auto* p = Platform::instance();
                if (p) outputs = p->getOutputs();
                log_msg = "Changed: " + out->name();
            });
        });
    }
};

// ── Monitor Card ─────────────────────────────────────────────────

WidgetPtr MonitorDemoState::buildCard(const std::shared_ptr<Output>& out, int idx) {
    bool selected   = (idx == selected_idx);
    bool is_primary = out->isPrimary();
    const auto& cm  = out->currentMode();

    std::string phys_str =
        std::to_string(cm.width) + "\u00d7" + std::to_string(cm.height) +
        "  @  " + fmtRefresh(cm.refresh_rate_mHz);

    std::string scale_str = fmtScale(out->scaleFactor(), out->fractionalScale());

    // Name line
    auto name_text = text({
        .text = out->name().empty() ? "Unknown" : out->name(),
        .color = selected ? Theme::primary : Theme::text_primary,
        .font_size = 13.0f,
        .font_weight = FontWeight::Bold,
    });

    // Primary badge
    WidgetPtr badge_row;
    if (is_primary) {
        auto b = text({
            .text = "PRIMARY",
            .color = Theme::accent_green,
            .font_size = 9.0f,
            .font_weight = FontWeight::Bold,
        });
        auto bc = container({
            .color = 0x2010B981,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::symmetric(2.0f, 7.0f),
            .child = b,
        });
        badge_row = bc;
    }

    // Resolution
    auto res_label = text({
        .text = phys_str,
        .color = Theme::accent_cyan,
        .font_size = 11.0f,
    });

    // Scale
    auto scale_label = text({
        .text = "Scale: " + scale_str,
        .color = Theme::accent_purple,
        .font_size = 11.0f,
    });

    // Description
    std::string desc = out->description();
    if (desc.empty()) desc = out->make() + " " + out->model();
    if (desc.empty()) desc = "Monitor";
    auto desc_text = text({
        .text = desc,
        .color = Theme::text_secondary,
        .font_size = 10.5f,
    });

    // Physical size
    std::string phys_mm_str;
    if (out->physicalWidthMm() > 0) {
        double diag_mm = std::sqrt(
            static_cast<double>(out->physicalWidthMm()) * out->physicalWidthMm() +
            static_cast<double>(out->physicalHeightMm()) * out->physicalHeightMm());
        std::ostringstream oss;
        oss << out->physicalWidthMm() << "\u00d7" << out->physicalHeightMm()
            << " mm (~" << std::fixed << std::setprecision(1) << diag_mm / 25.4 << "\")";
        phys_mm_str = oss.str();
    } else {
        phys_mm_str = "Physical size: N/A";
    }
    auto phys_text = text({
        .text = phys_mm_str,
        .color = Theme::text_muted,
        .font_size = 10.5f,
    });

    // Rotation
    auto rot_text = text({
        .text = "Rotation: " + fmtTransform(out->transform()),
        .color = Theme::text_muted,
        .font_size = 10.5f,
    });

    // Card body
    std::vector<WidgetPtr> body_items;
    auto hdr_row = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .children = {
            name_text,
            badge_row ? badge_row : sizedBox(0.0f, 0.0f),
        }
    });
    body_items.push_back(hdr_row);
    body_items.push_back(desc_text);
    body_items.push_back(sizedBox(0.0f, 4.0f));
    body_items.push_back(res_label);
    body_items.push_back(scale_label);
    body_items.push_back(phys_text);
    body_items.push_back(rot_text);

    auto card_col = column({
        .justify_content = Justify::Start,
        .align_items = Align::Start,
        .children = std::move(body_items),
    });

    auto card_box = container({
        .color = Theme::bg_card,
        .border_radius = BorderRadius::circular(12.0f),
        .border = Border(selected ? Theme::border_active : Theme::border_subtle, selected ? 1.5f : 1.0f),
        .width = StyleValue::point(360.0f),
        .padding = StyleInsets::all(14.0f),
        .child = card_col,
    });

    // Wrap in GestureDetector
    int cap_idx = idx;
    return gestureDetector({
        .key = Key::string("out_card_" + std::to_string(idx)),
        .child = card_box,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .cursor_type = SystemCursor::Pointer,
        .on_tap = [this, cap_idx]() {
            setState([this, cap_idx]() { selected_idx = cap_idx; });
        },
    });
}

// ── Mode Panel ────────────────────────────────────────────────────

WidgetPtr MonitorDemoState::buildModePanel(const std::shared_ptr<Output>& out) {
    auto title = text({
        .text = "Available Modes (" + out->name() + ")",
        .color = Theme::text_secondary,
        .font_size = 12.0f,
        .font_weight = FontWeight::Bold,
    });

    std::vector<WidgetPtr> rows;
    rows.push_back(title);
    rows.push_back(sizedBox(0.0f, 6.0f));

    const auto& modes = out->modes();
    int shown = 0;
    for (const auto& m : modes) {
        if (shown >= 10) break;
        std::string label =
            std::to_string(m.width) + "\u00d7" + std::to_string(m.height) +
            "  " + fmtRefresh(m.refresh_rate_mHz);
        if (m.is_current)   label += "  \u2190 current";
        if (m.is_preferred && !m.is_current) label += "  \u2605";

        auto lbl = text({
            .text = label,
            .color = m.is_current ? Theme::accent_green : Theme::text_muted,
            .font_size = 11.0f,
        });
        rows.push_back(lbl);
        ++shown;
    }
    if (static_cast<int>(modes.size()) > shown) {
        auto more = text({
            .text = "\u2026 and " + std::to_string(modes.size() - shown) + " more",
            .color = Theme::text_muted,
            .font_size = 10.5f,
        });
        rows.push_back(more);
    }

    auto col = column({
        .justify_content = Justify::Start,
        .align_items = Align::Start,
        .children = std::move(rows),
    });
    auto box = container({
        .color = Theme::bg_card,
        .border_radius = BorderRadius::circular(12.0f),
        .border = Border(Theme::border_subtle, 1.0f),
        .width = StyleValue::point(260.0f),
        .padding = StyleInsets::all(14.0f),
        .child = col,
    });
    return box;
}

// ── Main Build ─────────────────────────────────────────────────────

WidgetPtr MonitorDemoState::build(BuildContext&) {
    // Left: output cards
    std::vector<WidgetPtr> cards;
    auto list_title = text({
        .text = "Connected Monitors",
        .color = Theme::text_secondary,
        .font_size = 12.0f,
        .font_weight = FontWeight::Bold,
    });
    cards.push_back(list_title);
    cards.push_back(sizedBox(0.0f, 8.0f));

    if (outputs.empty()) {
        auto empty_label = text({
            .text = "No outputs detected",
            .color = Theme::text_muted,
            .font_size = 12.0f,
        });
        auto empty_box = container({
            .color = Theme::bg_card,
            .border_radius = BorderRadius::circular(12.0f),
            .width = StyleValue::point(360.0f),
            .padding = StyleInsets::all(20.0f),
            .child = empty_label,
        });
        cards.push_back(empty_box);
    } else {
        for (int i = 0; i < static_cast<int>(outputs.size()); ++i) {
            cards.push_back(buildCard(outputs[i], i));
            if (i + 1 < static_cast<int>(outputs.size()))
                cards.push_back(sizedBox(0.0f, 8.0f));
        }
    }

    auto left_col = column({
        .justify_content = Justify::Start,
        .align_items = Align::Start,
        .children = std::move(cards),
    });

    // Right: mode panel
    WidgetPtr right_panel;
    if (!outputs.empty() && selected_idx < static_cast<int>(outputs.size())) {
        right_panel = buildModePanel(outputs[selected_idx]);
    } else {
        auto ph = text({
            .text = "Select a monitor",
            .color = Theme::text_muted,
            .font_size = 12.0f,
        });
        auto ph_box = container({
            .color = Theme::bg_card,
            .border_radius = BorderRadius::circular(12.0f),
            .width = StyleValue::point(260.0f),
            .padding = StyleInsets::all(20.0f),
            .child = ph,
        });
        right_panel = ph_box;
    }

    auto content_row = row({
        .justify_content = Justify::Start,
        .align_items = Align::Start,
        .children = {
            left_col,
            sizedBox(16.0f, 0.0f),
            right_panel,
        }
    });

    // Header
    auto title = text({
        .text = "\u2b1b  Output & Monitor Manager",
        .color = Theme::primary,
        .font_size = 17.0f,
        .font_weight = FontWeight::Bold,
    });

    auto sub = text({
        .text = "Live display topology \u2014 Wayland: wl_output + zxdg_output_v1  |  X11: XRandR",
        .color = Theme::text_muted,
        .font_size = 10.5f,
    });

    // Status bar
    std::string count_str = std::to_string(outputs.size()) + " output(s) connected";
    auto log_label = text({
        .text = log_msg,
        .color = Theme::text_secondary,
        .font_size = 10.5f,
    });
    auto count_label = text({
        .text = count_str,
        .color = Theme::text_muted,
        .font_size = 10.5f,
    });

    auto status_inner = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .children = { log_label, count_label }
    });
    auto status_box = container({
        .color = Theme::bg_badge,
        .border_radius = BorderRadius::circular(8.0f),
        .border = Border(Theme::border_subtle, 1.0f),
        .width = StyleValue::point(680.0f),
        .padding = StyleInsets::symmetric(8.0f, 14.0f),
        .child = status_inner,
    });

    auto root_col = column({
        .justify_content = Justify::Start,
        .align_items = Align::Start,
        .children = {
            title,
            sizedBox(0.0f, 4.0f),
            sub,
            sizedBox(0.0f, 14.0f),
            content_row,
            sizedBox(0.0f, 14.0f),
            status_box,
        }
    });

    auto root_box = container({
        .color = Theme::bg_base,
        .padding = StyleInsets::all(20.0f),
        .child = root_col,
    });
    return root_box;
}

// ════════════════════════════════════════════════════════════════
// StatefulWidget Wrapper
// ════════════════════════════════════════════════════════════════

class MonitorDemoApp : public StatefulWidget {
public:
    MonitorDemoApp() : StatefulWidget(Key::string("monitor_demo_root")) {}

    std::unique_ptr<State> createState() override {
        return std::make_unique<MonitorDemoState>();
    }

    [[nodiscard]] std::string_view typeName() const override { return "MonitorDemoApp"; }
};

// ════════════════════════════════════════════════════════════════
// Entry Point
// ════════════════════════════════════════════════════════════════

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Platform \u2014 Output & Monitor Demo          \n";
    std::cout << "  Wayland: wl_output + zxdg_output_v1           \n";
    std::cout << "  X11:     XRandR                                \n";
    std::cout << "================================================\n";

    // Print discovered outputs to console
    if (auto* platform = Platform::instance()) {
        auto outputs = platform->getOutputs();
        std::cout << "\n[ENKI] Discovered " << outputs.size() << " output(s):\n";
        for (const auto& out : outputs) {
            const auto& cm = out->currentMode();
            std::cout << "  \u2022 " << out->name();
            if (!out->description().empty())
                std::cout << "  (" << out->description() << ")";
            std::cout << "  " << cm.width << "\u00d7" << cm.height
                      << " @ " << fmtRefresh(cm.refresh_rate_mHz)
                      << "  scale=" << fmtScale(out->scaleFactor(), out->fractionalScale());
            if (out->isPrimary()) std::cout << "  [PRIMARY]";
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    AppConfig config;
    config.title  = "ENKI \u2014 Monitor Demo";
    config.width  = 720;
    config.height = 580;
    config.window_mode = WindowMode::Normal;
    config.vsync  = true;

    return runApp(std::make_shared<MonitorDemoApp>(), config);
}
