/// @file main.cpp
/// @brief ENKI Engine — Flow Widget Showcase Demo (Roadmap v0.2.0 Section 11: Layout — Extended).
///
/// Demonstrates the immense power of the Flow widget:
///   1. Zero-reflow / Zero-relayout architecture for silky-smooth animations at .
///   2. Dynamic Matrix4 transformations (Translation, Rotation, Scale, Opacity).
///   3. Accurate hit testing through inverse matrix mapping.
///   4. Strict designated initializer syntax with unique Key on every element.
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/flow.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Custom Flow Delegates for Showcase Modes
// ════════════════════════════════════════════════════════════════

/// @brief Delegate 1: Radial Speed Dial / Fan-out Menu
class RadialMenuFlowDelegate : public FlowDelegate {
public:
    float radius;
    float start_angle_deg;
    float sweep_angle_deg;
    float open_progress; // 0.0 (closed) to 1.0 (fully open)
    Point center_origin;

    RadialMenuFlowDelegate(float r, float start_deg, float sweep_deg, float prog, Point origin)
        : radius(r), start_angle_deg(start_deg), sweep_angle_deg(sweep_deg),
          open_progress(prog), center_origin(origin) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    BoxConstraints getConstraintsForChild(size_t index, const BoxConstraints& /*constraints*/) override {
        // Center hub button is 64x64, orbital items are 52x52
        if (index == 0) {
            return BoxConstraints::tight(Size{64.0f, 64.0f});
        }
        return BoxConstraints::tight(Size{52.0f, 52.0f});
    }

    void paintChildren(FlowPaintingContext& context) override {
        if (context.childCount() == 0) return;

        // 1. Center hub button (Child 0)
        Size hub_sz = context.getChildSize(0);
        float hub_x = center_origin.x - (hub_sz.width / 2.0f);
        float hub_y = center_origin.y - (hub_sz.height / 2.0f);
        context.paintChild(0, Point{hub_x, hub_y}, 1.0f);

        // 2. Orbital action items (Children 1..N-1)
        size_t action_count = context.childCount() - 1;
        if (action_count == 0 || open_progress <= 0.001f) return;

        float current_radius = radius * open_progress;
        float start_rad = start_angle_deg * 3.14159265f / 180.0f;
        float sweep_rad = sweep_angle_deg * 3.14159265f / 180.0f;

        float step = (action_count > 1) ? (sweep_rad / static_cast<float>(action_count - 1)) : 0.0f;

        for (size_t i = 1; i < context.childCount(); ++i) {
            size_t idx = i - 1;
            float angle = start_rad + static_cast<float>(idx) * step;

            Size child_sz = context.getChildSize(i);
            float target_x = center_origin.x + current_radius * std::cos(angle) - (child_sz.width / 2.0f);
            float target_y = center_origin.y + current_radius * std::sin(angle) - (child_sz.height / 2.0f);

            // Subtle rotation and scale entrance
            float scale_val = 0.4f + (0.6f * open_progress);
            float rot_rad = (1.0f - open_progress) * 1.57f;

            auto transform = Matrix4::translation(target_x, target_y)
                           * Matrix4::rotationZ(rot_rad)
                           * Matrix4::scale(scale_val, scale_val);

            float opacity = std::clamp(open_progress * 1.2f, 0.0f, 1.0f);
            context.paintChild(i, transform, opacity);
        }
    }

    bool shouldRelayout(const FlowDelegate& old) const override {
        const auto* o = dynamic_cast<const RadialMenuFlowDelegate*>(&old);
        return !o;
    }

    bool shouldRepaint(const FlowDelegate& old) const override {
        const auto* o = dynamic_cast<const RadialMenuFlowDelegate*>(&old);
        return !o || o->radius != radius || o->start_angle_deg != start_angle_deg ||
               o->sweep_angle_deg != sweep_angle_deg || o->open_progress != open_progress ||
               o->center_origin.x != center_origin.x || o->center_origin.y != center_origin.y;
    }
};

/// @brief Delegate 2: 3D Depth Card Cascade
class CascadingCardsFlowDelegate : public FlowDelegate {
public:
    float spread_x;
    float spread_y;
    float card_rotation_deg;
    float scale_falloff;
    int selected_card;

    CascadingCardsFlowDelegate(float sx, float sy, float rot, float sf, int sel)
        : spread_x(sx), spread_y(sy), card_rotation_deg(rot), scale_falloff(sf), selected_card(sel) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    BoxConstraints getConstraintsForChild(size_t /*index*/, const BoxConstraints& /*constraints*/) override {
        return BoxConstraints::tight(Size{190.0f, 250.0f});
    }

    void paintChildren(FlowPaintingContext& context) override {
        size_t count = context.childCount();
        if (count == 0) return;

        float start_x = 70.0f;
        float start_y = 65.0f;

        for (size_t i = 0; i < count; ++i) {
            float fi = static_cast<float>(i);
            float x = start_x + (fi * spread_x);
            float y = start_y + (fi * spread_y);

            float scale = 1.0f - (fi * scale_falloff);
            if (scale < 0.6f) scale = 0.6f;

            float rot_rad = (fi * card_rotation_deg) * 3.14159265f / 180.0f;
            float opacity = 1.0f - (fi * 0.08f);
            if (opacity < 0.3f) opacity = 0.3f;

            // Elevate selected card
            if (static_cast<int>(i) == selected_card) {
                y -= 25.0f;
                scale += 0.06f;
                opacity = 1.0f;
            }

            auto transform = Matrix4::translation(x, y)
                           * Matrix4::rotationZ(rot_rad)
                           * Matrix4::scale(scale, scale);

            context.paintChild(i, transform, opacity);
        }
    }

    bool shouldRepaint(const FlowDelegate& old) const override {
        const auto* o = dynamic_cast<const CascadingCardsFlowDelegate*>(&old);
        return !o || o->spread_x != spread_x || o->spread_y != spread_y ||
               o->card_rotation_deg != card_rotation_deg || o->scale_falloff != scale_falloff ||
               o->selected_card != selected_card;
    }
};

/// @brief Delegate 3: Mathematical Sine Wave / Ribbon Flow
class WaveRibbonFlowDelegate : public FlowDelegate {
public:
    float amplitude;
    float frequency;
    float phase;
    float item_spacing;

    WaveRibbonFlowDelegate(float amp, float freq, float ph, float sp)
        : amplitude(amp), frequency(freq), phase(ph), item_spacing(sp) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    BoxConstraints getConstraintsForChild(size_t /*index*/, const BoxConstraints& /*constraints*/) override {
        return BoxConstraints::tight(Size{72.0f, 72.0f});
    }

    void paintChildren(FlowPaintingContext& context) override {
        size_t count = context.childCount();
        if (count == 0) return;

        float mid_y = context.size().height / 2.0f - 36.0f;
        float start_x = 40.0f;

        for (size_t i = 0; i < count; ++i) {
            float x = start_x + (static_cast<float>(i) * item_spacing);
            float angle = (x * frequency) + phase;
            float y = mid_y + (amplitude * std::sin(angle));

            // Dynamic rotation tangential to the sine curve
            float slope = amplitude * frequency * std::cos(angle);
            float rot_rad = std::atan(slope) * 0.5f;

            auto transform = Matrix4::translation(x, y)
                           * Matrix4::rotationZ(rot_rad);

            float opacity = 0.7f + 0.3f * std::cos(angle);
            context.paintChild(i, transform, opacity);
        }
    }

    bool shouldRepaint(const FlowDelegate& old) const override {
        const auto* o = dynamic_cast<const WaveRibbonFlowDelegate*>(&old);
        return !o || o->amplitude != amplitude || o->frequency != frequency ||
               o->phase != phase || o->item_spacing != item_spacing;
    }
};

// ════════════════════════════════════════════════════════════════
// Application & State Management
// ════════════════════════════════════════════════════════════════

enum class ShowcaseTab {
    RadialMenu,
    CascadingCards,
    WaveRibbon,
};

class FlowDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FlowDemoApp"; }
};

class FlowDemoState : public State {
private:
    ShowcaseTab current_tab_ = ShowcaseTab::RadialMenu;

    // Mode 1 State: Radial Speed Dial
    bool  radial_open_       = true;
    float radial_radius_     = 130.0f;
    float radial_sweep_deg_  = 300.0f;
    int   radial_item_count_ = 5;

    // Mode 2 State: Cascading Cards
    float cascade_spread_x_  = 75.0f;
    float cascade_spread_y_  = 18.0f;
    float cascade_rot_deg_   = -3.5f;
    int   selected_card_     = 0;

    // Mode 3 State: Wave Ribbon
    float wave_amplitude_    = 70.0f;
    float wave_frequency_    = 0.012f;
    float wave_phase_        = 0.0f;
    float wave_spacing_      = 85.0f;

    // Global Status
    std::string status_log_  = "Ready • Interactive Flow Widget Active";
    int interaction_count_   = 0;

public:
    void setTab(ShowcaseTab tab) {
        setState([this, tab]() {
            current_tab_ = tab;
            status_log_ = "Switched to Mode: " + tabName(tab);
            interaction_count_++;
        });
    }

    std::string tabName(ShowcaseTab tab) const {
        switch (tab) {
            case ShowcaseTab::RadialMenu:     return "Radial Speed Dial";
            case ShowcaseTab::CascadingCards: return "3D Depth Cascade";
            case ShowcaseTab::WaveRibbon:     return "Sine Wave Ribbon";
        }
        return "Unknown";
    }

    // ── Helper UI Builders ──────────────────────────────────────

    WidgetPtr makeIconButton(std::string id_key, std::string label, uint32_t bg_color,
                             uint32_t border_color, float sz, std::function<void()> on_click) {
        return gestureDetector({
            .key = Key::string("gd_" + id_key),
            .child = container({
                .color = bg_color,
                .border_radius = BorderRadius::circular(sz / 2.0f),
                .border = Border(border_color, 1.5f),
                .box_shadow = { BoxShadow(0x30000000, {0.0f, 4.0f}, 8.0f, 0.0f) },
                .align = Alignment::Center,
                .width = StyleValue::point(sz),
                .height = StyleValue::point(sz),
                .child = text({
                    .text = label,
                    .color = 0xFFFFFFFF,
                    .font_size = sz * 0.28f,
                    .font_weight = FontWeight::Bold,
                    .text_align = TextAlign::Center,
                    .key = Key::string("txt_" + id_key),
                }),
                .key = Key::string("box_" + id_key),
            }),
            .on_tap = [this, label, on_click]() {
                if (on_click) on_click();
                setState([this, label]() {
                    status_log_ = "Triggered: " + label;
                    interaction_count_++;
                });
            },
        });
    }

    WidgetPtr makeTabButton(ShowcaseTab tab, std::string title, std::string key_prefix) {
        bool is_active = (current_tab_ == tab);
        return gestureDetector({
            .key = Key::string(key_prefix + "_gd"),
            .child = container({
                .color = is_active ? 0xFF1E293B : 0xFF0D1526,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(is_active ? 0xFF6366F1 : 0xFF1E293B, is_active ? 1.5f : 1.0f),
                .align = Alignment::Center,
                .padding = StyleInsets::symmetric(9.0f, 18.0f),
                .child = text({
                    .text = title,
                    .color = is_active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 13.0f,
                    .font_weight = is_active ? FontWeight::Bold : FontWeight::Normal,
                    .key = Key::string(key_prefix + "_txt"),
                }),
                .key = Key::string(key_prefix + "_box"),
            }),
            .on_tap = [this, tab]() { setTab(tab); },
        });
    }

    WidgetPtr makeActionButton(std::string text_val, std::string id_key, std::function<void()> on_tap) {
        return gestureDetector({
            .key = Key::string("gd_btn_" + id_key),
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(0xFF334155, 1.0f),
                .align = Alignment::Center,
                .padding = StyleInsets::symmetric(6.0f, 12.0f),
                .child = text({
                    .text = text_val,
                    .color = 0xFFE2E8F0,
                    .font_size = 11.5f,
                    .font_weight = FontWeight::Medium,
                    .key = Key::string("txt_btn_" + id_key),
                }),
                .key = Key::string("box_btn_" + id_key),
            }),
            .on_tap = [this, on_tap]() {
                if (on_tap) on_tap();
                setState([this]() { interaction_count_++; });
            },
        });
    }

    // ── Mode 1 Scene: Radial Speed Dial ─────────────────────────

    WidgetPtr buildRadialMenuScene() {
        Point center{320.0f, 190.0f};

        auto delegate = std::make_shared<RadialMenuFlowDelegate>(
            radial_radius_,
            210.0f,
            radial_sweep_deg_,
            radial_open_ ? 1.0f : 0.0f,
            center
        );

        std::vector<WidgetPtr> children;

        // Child 0: Center Hub Action Button
        children.push_back(makeIconButton(
            "center_hub",
            radial_open_ ? "✕" : "＋",
            radial_open_ ? 0xFFDC2626 : 0xFF6366F1,
            radial_open_ ? 0xFFF87171 : 0xFF818CF8,
            64.0f,
            [this]() {
                setState([this]() {
                    radial_open_ = !radial_open_;
                    status_log_ = radial_open_ ? "Radial Menu: Opened" : "Radial Menu: Closed";
                });
            }
        ));

        // Child 1..N: Orbital Items
        struct OrbitalItem {
            std::string label;
            uint32_t color;
            uint32_t border;
        };
        std::vector<OrbitalItem> items = {
            {"📷", 0xFF0284C7, 0xFF38BDF8},
            {"🎨", 0xFF7C3AED, 0xFFA78BFA},
            {"🎵", 0xFF059669, 0xFF34D399},
            {"📍", 0xFFD97706, 0xFFFBBF24},
            {"⚙", 0xFF475569, 0xFF94A3B8},
            {"💬", 0xFFE11D48, 0xFFFB7185},
        };

        for (int i = 0; i < radial_item_count_ && i < static_cast<int>(items.size()); ++i) {
            std::string id = "orbital_" + std::to_string(i);
            children.push_back(makeIconButton(
                id,
                items[static_cast<size_t>(i)].label,
                items[static_cast<size_t>(i)].color,
                items[static_cast<size_t>(i)].border,
                52.0f,
                [this, i, items]() {
                    status_log_ = "Orbital action clicked: " + items[static_cast<size_t>(i)].label;
                }
            ));
        }

        return flow({
            .key = Key::string("radial_flow"),
            .delegate = delegate,
            .children = std::move(children),
        });
    }

    // ── Mode 2 Scene: 3D Depth Card Cascade ─────────────────────

    WidgetPtr buildCascadingCardsScene() {
        auto delegate = std::make_shared<CascadingCardsFlowDelegate>(
            cascade_spread_x_,
            cascade_spread_y_,
            cascade_rot_deg_,
            0.05f,
            selected_card_
        );

        struct CardMeta {
            std::string title;
            std::string tag;
            uint32_t gradient_start;
            uint32_t gradient_end;
        };

        std::vector<CardMeta> cards = {
            {"Quantum Core", "SKIA 2D", 0xFF4F46E5, 0xFF06B6D4},
            {"Zero Reflow", "ANU FLEX", 0xFF059669, 0xFF10B981},
            {"Flow Layout", "TRANSFORMS", 0xFFD97706, 0xFFF59E0B},
            {"Matrix4 Engine", "MATH 3D", 0xFF9333EA, 0xFFC084FC},
            {"Ultra FPS", "800+ HZ", 0xFFE11D48, 0xFFFB7185},
        };

        std::vector<WidgetPtr> children;
        for (size_t i = 0; i < cards.size(); ++i) {
            int card_idx = static_cast<int>(i);
            bool is_sel = (selected_card_ == card_idx);

            children.push_back(gestureDetector({
                .key = Key::string("card_gd_" + std::to_string(i)),
                .child = container({
                    .color = is_sel ? 0xFF1E293B : 0xFF0F172A,
                    .border_radius = BorderRadius::circular(14.0f),
                    .border = Border(is_sel ? 0xFF38BDF8 : 0xFF334155, is_sel ? 2.0f : 1.0f),
                    .box_shadow = {
                        BoxShadow(is_sel ? 0x6038BDF8 : 0x40000000, {0.0f, is_sel ? 8.0f : 4.0f}, is_sel ? 20.0f : 10.0f, 0.0f)
                    },
                    .width = StyleValue::point(190.0f),
                    .height = StyleValue::point(250.0f),
                    .padding = StyleInsets::all(16.0f),
                    .child = column({
                        .justify_content = Justify::SpaceBetween,
                        .children = {
                            column({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    container({
                                        .color = cards[i].gradient_start,
                                        .border_radius = BorderRadius::circular(6.0f),
                                        .padding = StyleInsets::symmetric(4.0f, 8.0f),
                                        .child = text({
                                            .text = cards[i].tag,
                                            .color = 0xFFFFFFFF,
                                            .font_size = 9.5f,
                                            .font_weight = FontWeight::Bold,
                                            .key = Key::string("tag_txt_" + std::to_string(i)),
                                        }),
                                        .key = Key::string("tag_box_" + std::to_string(i)),
                                    }),
                                    text({
                                        .text = cards[i].title,
                                        .color = 0xFFFFFFFF,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("title_txt_" + std::to_string(i)),
                                    }),
                                    text({
                                        .text = "Matrix4 affine projection with zero-reflow paint passes.",
                                        .color = 0xFF94A3B8,
                                        .font_size = 11.0f,
                                        .key = Key::string("desc_txt_" + std::to_string(i)),
                                    }),
                                },
                                .key = Key::string("card_top_col_" + std::to_string(i)),
                            }),
                            container({
                                .color = is_sel ? 0xFF0284C7 : 0xFF1E293B,
                                .border_radius = BorderRadius::circular(8.0f),
                                .align = Alignment::Center,
                                .padding = StyleInsets::symmetric(8.0f, 12.0f),
                                .child = text({
                                    .text = is_sel ? "★ SELECTED" : "SELECT CARD",
                                    .color = is_sel ? 0xFFFFFFFF : 0xFFCBD5E1,
                                    .font_size = 10.5f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("btn_txt_" + std::to_string(i)),
                                }),
                                .key = Key::string("btn_box_" + std::to_string(i)),
                            }),
                        },
                        .key = Key::string("card_col_" + std::to_string(i)),
                    }),
                    .key = Key::string("card_box_" + std::to_string(i)),
                }),
                .on_tap = [this, card_idx, cards]() {
                    setState([this, card_idx, cards]() {
                        selected_card_ = card_idx;
                        status_log_ = "Selected Card: " + cards[static_cast<size_t>(card_idx)].title;
                        interaction_count_++;
                    });
                },
            }));
        }

        return flow({
            .key = Key::string("cascade_flow"),
            .delegate = delegate,
            .children = std::move(children),
        });
    }

    // ── Mode 3 Scene: Mathematical Sine Wave Ribbon ─────────────

    WidgetPtr buildWaveRibbonScene() {
        auto delegate = std::make_shared<WaveRibbonFlowDelegate>(
            wave_amplitude_,
            wave_frequency_,
            wave_phase_,
            wave_spacing_
        );

        std::vector<WidgetPtr> children;
        for (int i = 0; i < 7; ++i) {
            std::string id = "wave_node_" + std::to_string(i);
            children.push_back(makeIconButton(
                id,
                "#" + std::to_string(i + 1),
                0xFF0F172A,
                0xFF06B6D4,
                72.0f,
                [this, i]() {
                    status_log_ = "Sine Wave Node #" + std::to_string(i + 1) + " clicked!";
                }
            ));
        }

        return flow({
            .key = Key::string("wave_flow"),
            .delegate = delegate,
            .children = std::move(children),
        });
    }

    // ── Controls Bar Builder ────────────────────────────────────

    WidgetPtr buildControls() {
        if (current_tab_ == ShowcaseTab::RadialMenu) {
            return row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({ .text = "RADIUS:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_rad") }),
                            makeActionButton("- 20px", "rad_minus", [this]() { radial_radius_ = std::max(80.0f, radial_radius_ - 20.0f); }),
                            text({ .text = std::to_string(static_cast<int>(radial_radius_)) + "px", .color = 0xFF38BDF8, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("val_rad") }),
                            makeActionButton("+ 20px", "rad_plus", [this]() { radial_radius_ = std::min(180.0f, radial_radius_ + 20.0f); }),
                        },
                        .key = Key::string("rad_ctrl_row"),
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({ .text = "SWEEP:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_swp") }),
                            makeActionButton("180°", "swp_180", [this]() { radial_sweep_deg_ = 180.0f; }),
                            makeActionButton("270°", "swp_270", [this]() { radial_sweep_deg_ = 270.0f; }),
                            makeActionButton("360°", "swp_360", [this]() { radial_sweep_deg_ = 360.0f; }),
                        },
                        .key = Key::string("swp_ctrl_row"),
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({ .text = "ITEMS:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_items") }),
                            makeActionButton("-", "items_minus", [this]() { radial_item_count_ = std::max(3, radial_item_count_ - 1); }),
                            text({ .text = std::to_string(radial_item_count_), .color = 0xFF38BDF8, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("val_items") }),
                            makeActionButton("+", "items_plus", [this]() { radial_item_count_ = std::min(6, radial_item_count_ + 1); }),
                        },
                        .key = Key::string("items_ctrl_row"),
                    }),
                },
                .key = Key::string("radial_ctrls_bar"),
            });
        }

        if (current_tab_ == ShowcaseTab::CascadingCards) {
            return row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({ .text = "SPREAD X:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_spx") }),
                            makeActionButton("- 10px", "spx_minus", [this]() { cascade_spread_x_ = std::max(40.0f, cascade_spread_x_ - 10.0f); }),
                            text({ .text = std::to_string(static_cast<int>(cascade_spread_x_)) + "px", .color = 0xFF38BDF8, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("val_spx") }),
                            makeActionButton("+ 10px", "spx_plus", [this]() { cascade_spread_x_ = std::min(110.0f, cascade_spread_x_ + 10.0f); }),
                        },
                        .key = Key::string("spx_ctrl_row"),
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({ .text = "CARD ROTATION:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_rot") }),
                            makeActionButton("0°", "rot_0", [this]() { cascade_rot_deg_ = 0.0f; }),
                            makeActionButton("-3.5°", "rot_neg", [this]() { cascade_rot_deg_ = -3.5f; }),
                            makeActionButton("+3.5°", "rot_pos", [this]() { cascade_rot_deg_ = 3.5f; }),
                        },
                        .key = Key::string("rot_ctrl_row"),
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            makeActionButton("PREV CARD", "sel_prev", [this]() { selected_card_ = (selected_card_ + 4) % 5; }),
                            makeActionButton("NEXT CARD", "sel_next", [this]() { selected_card_ = (selected_card_ + 1) % 5; }),
                        },
                        .key = Key::string("card_nav_row"),
                    }),
                },
                .key = Key::string("cascade_ctrls_bar"),
            });
        }

        // Wave Ribbon Controls
        return row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text({ .text = "AMPLITUDE:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_amp") }),
                        makeActionButton("30px", "amp_30", [this]() { wave_amplitude_ = 30.0f; }),
                        makeActionButton("70px", "amp_70", [this]() { wave_amplitude_ = 70.0f; }),
                        makeActionButton("110px", "amp_110", [this]() { wave_amplitude_ = 110.0f; }),
                    },
                    .key = Key::string("amp_ctrl_row"),
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text({ .text = "PHASE SHIFT:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_phase") }),
                        makeActionButton("◀ Left", "phase_left", [this]() { wave_phase_ -= 0.5f; }),
                        makeActionButton("Reset", "phase_reset", [this]() { wave_phase_ = 0.0f; }),
                        makeActionButton("Right ▶", "phase_right", [this]() { wave_phase_ += 0.5f; }),
                    },
                    .key = Key::string("phase_ctrl_row"),
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text({ .text = "SPACING:", .color = 0xFF94A3B8, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("lbl_spc") }),
                        makeActionButton("- 15px", "spc_minus", [this]() { wave_spacing_ = std::max(60.0f, wave_spacing_ - 15.0f); }),
                        text({ .text = std::to_string(static_cast<int>(wave_spacing_)) + "px", .color = 0xFF38BDF8, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("val_spc") }),
                        makeActionButton("+ 15px", "spc_plus", [this]() { wave_spacing_ = std::min(110.0f, wave_spacing_ + 15.0f); }),
                    },
                    .key = Key::string("spc_ctrl_row"),
                }),
            },
            .key = Key::string("wave_ctrls_bar"),
        });
    }

    WidgetPtr build(BuildContext& /*context*/) override {
        return container({
            .color = 0xFF060911,
            .padding = StyleInsets::symmetric(24.0f, 32.0f),
            .child = column({
                .children = {
                    // Header Bar
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            column({
                                .gap = StyleValue::point(4.0f),
                                .children = {
                                    text({
                                        .text = "FLOW LAYOUT SHOWCASE",
                                        .color = 0xFFFFFFFF,
                                        .font_size = 20.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("header_title_txt"),
                                    }),
                                    text({
                                        .text = "Roadmap v0.2.0 Section 11 (6 of 6 Completed • 100%)",
                                        .color = 0xFF64748B,
                                        .font_size = 12.0f,
                                        .key = Key::string("header_sub_txt"),
                                    }),
                                },
                                .key = Key::string("header_left_col"),
                            }),
                            container({
                                .color = 0xFF0D251A,
                                .border_radius = BorderRadius::circular(20.0f),
                                .border = Border(0xFF10B981, 1.0f),
                                .align = Alignment::Center,
                                .padding = StyleInsets::symmetric(6.0f, 14.0f),
                                .child = text({
                                    .text = "ENKI v0.2.0 • ",
                                    .color = 0xFF10B981,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("fps_badge_txt"),
                                }),
                                .key = Key::string("fps_badge_box"),
                            }),
                        },
                        .key = Key::string("top_header_row"),
                    }),

                    container({ .height = StyleValue::point(18.0f), .key = Key::string("sp_top") }),

                    // Navigation Tabs
                    row({
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            makeTabButton(ShowcaseTab::RadialMenu, "1. Radial Speed Dial", "radial"),
                            makeTabButton(ShowcaseTab::CascadingCards, "2. 3D Depth Cascade", "cascade"),
                            makeTabButton(ShowcaseTab::WaveRibbon, "3. Sine Wave Ribbon", "wave"),
                        },
                        .key = Key::string("tabs_row"),
                    }),

                    container({ .height = StyleValue::point(14.0f), .key = Key::string("sp_mid1") }),

                    // Parameter Controls Bar
                    container({
                        .color = 0xFF0B1120,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .padding = StyleInsets::symmetric(12.0f, 16.0f),
                        .child = buildControls(),
                        .key = Key::string("ctrls_box"),
                    }),

                    container({ .height = StyleValue::point(16.0f), .key = Key::string("sp_mid2") }),

                    // Main Flow Stage
                    container({
                        .color = 0xFF070B14,
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF1F2937, 1.0f),
                        .height = StyleValue::point(380.0f),
                        .child = (current_tab_ == ShowcaseTab::RadialMenu)
                            ? buildRadialMenuScene()
                            : (current_tab_ == ShowcaseTab::CascadingCards)
                                ? buildCascadingCardsScene()
                                : buildWaveRibbonScene(),
                        .key = Key::string("stage_box"),
                    }),

                    container({ .height = StyleValue::point(14.0f), .key = Key::string("sp_mid3") }),

                    // Live Status Bar
                    container({
                        .color = 0xFF0D1526,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .padding = StyleInsets::symmetric(10.0f, 16.0f),
                        .child = row({
                            .justify_content = Justify::SpaceBetween,
                            .align_items = Align::Center,
                            .children = {
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        text({ .text = "STATUS:", .color = 0xFF06B6D4, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("st_lbl") }),
                                        text({ .text = status_log_, .color = 0xFFE2E8F0, .font_size = 11.0f, .key = Key::string("st_val") }),
                                    },
                                    .key = Key::string("st_left_row"),
                                }),
                                text({
                                    .text = "Events Dispatched: " + std::to_string(interaction_count_),
                                    .color = 0xFF94A3B8,
                                    .font_size = 11.0f,
                                    .key = Key::string("st_cnt"),
                                }),
                            },
                            .key = Key::string("status_row"),
                        }),
                        .key = Key::string("box_status_bar"),
                    }),
                },
                .key = Key::string("root_col"),
            }),
            .key = Key::string("root_container"),
        });
    }
};

std::unique_ptr<State> FlowDemoApp::createState() {
    return std::make_unique<FlowDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — Flow Layout Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended (6/6)" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — Flow Layout Showcase Demo";
    config.width       = 1140;
    config.height      = 820;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<FlowDemoApp>(), config);
}
