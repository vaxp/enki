/// @file main.cpp
/// @brief ENKI Stack & Positioned Interactive Showcase Demo
///
/// Features demonstrated:
///   1. Notification badges on Avatars & Action icons with absolute negative/positive offsets.
///   2. Hero Card with Layered Gradients, Floating Status Chips, and absolute text overlays.
///   3. Staggered overlapping cards with Z-index hit-testing (clicking layers).
///   4. Interactive Fullscreen Modal Dialog overlay created via Positioned::fill().
///   5. Dynamic coordinate controls and interactive toggles.

#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/stack.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design System & Palette
// ════════════════════════════════════════════════════════════════

namespace Style {
    constexpr Color bg_dark       = 0xFF0B0F19;
    constexpr Color bg_card       = 0xFF161D2F;
    constexpr Color bg_card_light = 0xFF1F293D;
    constexpr Color border_subtle = 0x3038BDF8;
    constexpr Color border_bright = 0x8038BDF8;
    constexpr Color primary       = 0xFF6366F1;
    constexpr Color primary_light = 0xFF818CF8;
    constexpr Color cyan_neon     = 0xFF00E5FF;
    constexpr Color emerald       = 0xFF10B981;
    constexpr Color rose          = 0xFFF43F5E;
    constexpr Color amber         = 0xFFF59E0B;
    constexpr Color text_white    = 0xFFFFFFFF;
    constexpr Color text_muted    = 0xFF94A3B8;
}

// ════════════════════════════════════════════════════════════════
// Stack Demo State & App
// ════════════════════════════════════════════════════════════════

class StackDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "StackDemoApp"; }
    std::unique_ptr<State> createState() override;
};

class StackDemoState : public State {
public:
    int badge_count = 3;
    bool is_online = true;
    int active_layer_index = 2; // 0, 1, or 2
    bool show_modal = false;
    float badge_offset_x = -4.0f;
    float badge_offset_y = -4.0f;

    WidgetPtr build(BuildContext& context) override {
        // Main content tree
        auto content = container(column({
            buildHeader(),
            sizedBox(0, 18.0f),
            buildCardsGrid(),
            sizedBox(0, 18.0f),
            buildControlsRow(),
        }));
        content->paddingAll(24.0f).color(Style::bg_dark);

        // Root stack allows overlaying modal on top of entire window
        std::vector<WidgetPtr> root_children;
        root_children.push_back(Positioned::fill(content));

        if (show_modal) {
            root_children.push_back(buildModalOverlay());
        }

        auto root = stack(root_children);
        root->fit(StackFit::Expand);
        return root;
    }

private:
    WidgetPtr buildHeader() {
        auto title = text("ENKI — Stack & Positioned Layout Architecture");
        title->fontSize(22.0f).bold().color(Style::text_white);

        auto subtitle = text("Multi-layered 2.5D visual hierarchy with absolute offsets, alignment & reverse hit-testing");
        subtitle->fontSize(12.5f).color(Style::cyan_neon);

        return column({
            title,
            sizedBox(0, 4.0f),
            subtitle,
        });
    }

    WidgetPtr buildCardsGrid() {
        auto card1 = buildAvatarBadgeCard();
        auto card2 = buildHeroGradientCard();
        auto card3 = buildLayeredStackCard();

        return row(Justify::SpaceBetween, Align::Start, {
            card1,
            card2,
            card3,
        });
    }

    // ── Card 1: Avatar with Positioned Online & Counter Badges ──

    WidgetPtr buildAvatarBadgeCard() {
        auto card_title = text("1. Floating Badges");
        card_title->fontSize(14.0f).bold().color(Style::text_white);

        auto desc = text("Badges positioned at absolute coordinates with zero parent flow disruption.");
        desc->fontSize(11.0f).color(Style::text_muted);

        // Avatar container with centered icon
        auto avatar_icon = text("👤");
        avatar_icon->fontSize(32.0f);

        auto avatar = container(avatar_icon);
        avatar->size(72.0f, 72.0f)
              .align(Alignment::Center)
              .borderRadius(36.0f)
              .color(0xFF312E81)
              .border(Style::primary_light, 2.0f);

        // Online dot positioned bottom-right
        auto online_dot = container();
        online_dot->size(16.0f, 16.0f)
                  .borderRadius(8.0f)
                  .color(is_online ? Style::emerald : Style::amber)
                  .border(Style::bg_card, 2.5f);

        auto pos_online = positioned(online_dot);
        pos_online->bottom(2.0f).right(2.0f);

        // Count badge positioned top-right
        auto count_text = text(std::to_string(badge_count));
        count_text->fontSize(10.0f).bold().color(Style::text_white);

        auto count_pill = container(count_text);
        count_pill->paddingSymmetric(6.0f, 2.0f)
                  .borderRadius(10.0f)
                  .color(Style::rose)
                  .border(Style::bg_card, 2.0f);

        auto pos_count = positioned(count_pill);
        pos_count->top(badge_offset_y).right(badge_offset_x);

        // The Stack wrapping the avatar + badges
        auto avatar_stack = stack({
            avatar,
            pos_online,
            pos_count,
        });
        avatar_stack->width(76.0f).height(76.0f).clip(Clip::None);

        // Action button to increment badge
        auto btn_add_text = text("+ Badge Count");
        btn_add_text->fontSize(11.5f).bold().color(Style::text_white);

        auto btn_add_box = container(btn_add_text);
        btn_add_box->paddingSymmetric(12.0f, 7.0f)
                   .color(Style::primary)
                   .borderRadius(6.0f);

        auto btn_add = gestureDetector(Key::string("btn_add_badge"), btn_add_box);
        btn_add->onTap([this]() {
            setState([this]() {
                badge_count = (badge_count % 99) + 1;
            });
        }).cursor(SystemCursor::Pointer);

        // Toggle online status
        auto btn_toggle_text = text(is_online ? "Set Away" : "Set Online");
        btn_toggle_text->fontSize(11.5f).color(Style::cyan_neon);

        auto btn_toggle_box = container(btn_toggle_text);
        btn_toggle_box->paddingSymmetric(10.0f, 6.0f)
                      .color(Style::bg_card_light)
                      .borderRadius(6.0f)
                      .border(Style::border_subtle, 1.0f);

        auto btn_toggle = gestureDetector(Key::string("btn_toggle_online"), btn_toggle_box);
        btn_toggle->onTap([this]() {
            setState([this]() {
                is_online = !is_online;
            });
        }).cursor(SystemCursor::Pointer);

        auto avatar_centered = container(avatar_stack);
        avatar_centered->align(Alignment::Center);

        auto card_content = column({
            card_title,
            sizedBox(0, 6.0f),
            desc,
            sizedBox(0, 16.0f),
            avatar_centered,
            sizedBox(0, 16.0f),
            row(Justify::Center, Align::Center, {
                btn_add,
                sizedBox(8.0f, 0),
                btn_toggle,
            }),
        });

        auto c = container(card_content);
        c->width(310.0f)
         .height(280.0f)
         .paddingAll(16.0f)
         .color(Style::bg_card)
         .borderRadius(12.0f)
         .border(Style::border_subtle, 1.0f);
        return c;
    }

    // ── Card 2: Hero Feature Card with Layered Gradient & Chips ──

    WidgetPtr buildHeroGradientCard() {
        auto card_title = text("2. Layered Hero Card");
        card_title->fontSize(14.0f).bold().color(Style::text_white);

        auto desc = text("Multi-layer composition: Base image + dark gradient overlay + status tags.");
        desc->fontSize(11.0f).color(Style::text_muted);

        // Base background gradient layer
        auto base_bg = container();
        base_bg->gradient(GradientConfig::linear({0xFF1E1B4B, 0xFF4338CA, 0xFF0F172A}))
               .borderRadius(8.0f);

        // PRO status chip positioned top-right
        auto pro_text = text("PRO FEATURE");
        pro_text->fontSize(9.5f).bold().color(Style::bg_dark);

        auto pro_chip = container(pro_text);
        pro_chip->paddingSymmetric(8.0f, 3.0f)
                .borderRadius(4.0f)
                .color(Style::cyan_neon);

        auto pos_pro = positioned(pro_chip);
        pos_pro->top(10.0f).right(10.0f);

        // Rating pill positioned top-left
        auto star_text = text("★ 4.98");
        star_text->fontSize(10.0f).bold().color(Style::amber);

        auto star_pill = container(star_text);
        star_pill->paddingSymmetric(8.0f, 3.0f)
                 .borderRadius(4.0f)
                 .color(0xCC000000);

        auto pos_star = positioned(star_pill);
        pos_star->top(10.0f).left(10.0f);

        // Bottom text overlay positioned bottom-left
        auto hero_head = text("Hyper-Vector Pipeline");
        hero_head->fontSize(13.0f).bold().color(Style::text_white);

        auto hero_sub = text("0.3ms frame latency");
        hero_sub->fontSize(10.5f).color(Style::emerald);

        auto bottom_content = column({ hero_head, hero_sub });
        auto bottom_box = container(bottom_content);
        bottom_box->paddingAll(10.0f)
                  .gradient(GradientConfig::linear({0x00000000, 0xE60B0F19}));

        auto pos_bottom = positioned(bottom_box);
        pos_bottom->left(0.0f).right(0.0f).bottom(0.0f);

        // Hero Stack
        auto hero_stack = stack({
            Positioned::fill(base_bg),
            pos_pro,
            pos_star,
            pos_bottom,
        });
        hero_stack->width(278.0f).height(160.0f).clip(Clip::HardEdge);

        auto card_content = column({
            card_title,
            sizedBox(0, 6.0f),
            desc,
            sizedBox(0, 12.0f),
            hero_stack,
        });

        auto c = container(card_content);
        c->width(310.0f)
         .height(280.0f)
         .paddingAll(16.0f)
         .color(Style::bg_card)
         .borderRadius(12.0f)
         .border(Style::border_subtle, 1.0f);
        return c;
    }

    // ── Card 3: Overlapping Z-Index Hit-Testing Layers ──

    WidgetPtr buildLayeredStackCard() {
        auto card_title = text("3. Z-Index Hit-Testing");
        card_title->fontSize(14.0f).bold().color(Style::text_white);

        auto desc = text("Click overlapping layers: Topmost layer receives events first.");
        desc->fontSize(11.0f).color(Style::text_muted);

        // Layer 0 (Bottom)
        auto l0_text = text("Layer 0 (Base)");
        l0_text->fontSize(10.5f).bold().color(Style::text_muted);
        auto l0_box = container(l0_text);
        l0_box->paddingAll(8.0f)
              .color(active_layer_index == 0 ? Style::primary : Style::bg_card_light)
              .borderRadius(8.0f)
              .border(Style::border_subtle, 1.0f);
        auto l0_gesture = gestureDetector(Key::string("layer_0"), l0_box);
        l0_gesture->onTap([this]() {
            setState([this]() { active_layer_index = 0; });
        }).cursor(SystemCursor::Pointer);
        auto pos_l0 = positioned(l0_gesture);
        pos_l0->top(0.0f).left(0.0f).width(160.0f).height(70.0f);

        // Layer 1 (Middle)
        auto l1_text = text("Layer 1 (Mid)");
        l1_text->fontSize(10.5f).bold().color(Style::text_white);
        auto l1_box = container(l1_text);
        l1_box->paddingAll(8.0f)
              .color(active_layer_index == 1 ? Style::primary : 0xFF2D3748)
              .borderRadius(8.0f)
              .border(Style::cyan_neon, active_layer_index == 1 ? 1.5f : 0.5f);
        auto l1_gesture = gestureDetector(Key::string("layer_1"), l1_box);
        l1_gesture->onTap([this]() {
            setState([this]() { active_layer_index = 1; });
        }).cursor(SystemCursor::Pointer);
        auto pos_l1 = positioned(l1_gesture);
        pos_l1->top(30.0f).left(40.0f).width(160.0f).height(70.0f);

        // Layer 2 (Top)
        auto l2_text = text("Layer 2 (Front)");
        l2_text->fontSize(10.5f).bold().color(Style::text_white);
        auto l2_box = container(l2_text);
        l2_box->paddingAll(8.0f)
              .color(active_layer_index == 2 ? Style::primary : 0xFF4A5568)
              .borderRadius(8.0f)
              .border(Style::cyan_neon, active_layer_index == 2 ? 1.5f : 0.5f);
        auto l2_gesture = gestureDetector(Key::string("layer_2"), l2_box);
        l2_gesture->onTap([this]() {
            setState([this]() { active_layer_index = 2; });
        }).cursor(SystemCursor::Pointer);
        auto pos_l2 = positioned(l2_gesture);
        pos_l2->top(60.0f).left(80.0f).width(160.0f).height(70.0f);

        auto overlap_stack = stack({
            pos_l0,
            pos_l1,
            pos_l2,
        });
        overlap_stack->width(250.0f).height(140.0f).clip(Clip::None);

        auto active_label = text("Active Clicked: Layer #" + std::to_string(active_layer_index));
        active_label->fontSize(11.0f).bold().color(Style::cyan_neon);

        auto card_content = column({
            card_title,
            sizedBox(0, 6.0f),
            desc,
            sizedBox(0, 8.0f),
            overlap_stack,
            sizedBox(0, 4.0f),
            active_label,
        });

        auto c = container(card_content);
        c->width(310.0f)
         .height(280.0f)
         .paddingAll(16.0f)
         .color(Style::bg_card)
         .borderRadius(12.0f)
         .border(Style::border_subtle, 1.0f);
        return c;
    }

    // ── Controls Row (Modal Trigger & Offset Sliders) ──

    WidgetPtr buildControlsRow() {
        auto btn_modal_text = text("🚀 Open Stack Modal Overlay");
        btn_modal_text->fontSize(13.0f).bold().color(Style::text_white);

        auto btn_modal_box = container(btn_modal_text);
        btn_modal_box->paddingSymmetric(18.0f, 10.0f)
                     .color(Style::primary)
                     .borderRadius(8.0f)
                     .border(Style::cyan_neon, 1.0f);

        auto btn_modal = gestureDetector(Key::string("btn_modal"), btn_modal_box);
        btn_modal->onTap([this]() {
            setState([this]() {
                show_modal = true;
            });
        }).cursor(SystemCursor::Pointer);

        auto reset_btn_text = text("Reset State");
        reset_btn_text->fontSize(12.0f).color(Style::text_muted);
        auto reset_btn_box = container(reset_btn_text);
        reset_btn_box->paddingSymmetric(14.0f, 10.0f)
                     .color(Style::bg_card)
                     .borderRadius(8.0f)
                     .border(Style::border_subtle, 1.0f);
        auto reset_btn = gestureDetector(Key::string("btn_reset_all"), reset_btn_box);
        reset_btn->onTap([this]() {
            setState([this]() {
                badge_count = 3;
                is_online = true;
                active_layer_index = 2;
                show_modal = false;
            });
        }).cursor(SystemCursor::Pointer);

        return row(Justify::Start, Align::Center, {
            btn_modal,
            sizedBox(12.0f, 0),
            reset_btn,
        });
    }

    // ── Fullscreen Stack Modal Overlay (Positioned::fill) ──

    WidgetPtr buildModalOverlay() {
        // Semi-transparent backdrop
        auto backdrop = container();
        backdrop->color(0xB3000000); // 70% black backdrop

        auto backdrop_gesture = gestureDetector(Key::string("modal_backdrop"), backdrop);
        backdrop_gesture->onTap([this]() {
            setState([this]() { show_modal = false; });
        });

        // Dialog Content Card
        auto modal_title = text("🌟 Stack Overlay Modal Dialog");
        modal_title->fontSize(18.0f).bold().color(Style::text_white);

        auto modal_body = text("This entire overlay is rendered via Positioned::fill() on the root Stack.\n"
                               "It intercepts all pointer clicks and blocks background interaction seamlessly.");
        modal_body->fontSize(12.0f).color(Style::text_muted);

        auto close_btn_text = text("Close Modal");
        close_btn_text->fontSize(12.0f).bold().color(Style::text_white);

        auto close_btn_box = container(close_btn_text);
        close_btn_box->paddingSymmetric(16.0f, 8.0f)
                     .color(Style::rose)
                     .borderRadius(6.0f);

        auto close_btn = gestureDetector(Key::string("modal_close"), close_btn_box);
        close_btn->onTap([this]() {
            setState([this]() { show_modal = false; });
        }).cursor(SystemCursor::Pointer);

        auto dialog_card = container(column({
            modal_title,
            sizedBox(0, 10.0f),
            modal_body,
            sizedBox(0, 18.0f),
            row(Justify::End, Align::Center, { close_btn }),
        }));
        dialog_card->width(460.0f)
                   .paddingAll(20.0f)
                   .color(Style::bg_card)
                   .borderRadius(14.0f)
                   .border(Style::cyan_neon, 1.5f);

        // Center the dialog card in the modal overlay stack
        auto modal_stack = stack(Alignment::Center, {
            Positioned::fill(backdrop_gesture),
            dialog_card,
        });
        modal_stack->fit(StackFit::Expand);

        return Positioned::fill(modal_stack);
    }
};

std::unique_ptr<State> StackDemoApp::createState() {
    return std::make_unique<StackDemoState>();
}

// ════════════════════════════════════════════════════════════════
// Main Entry
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    AppConfig config;
    config.title                    = "ENKI — Stack & Positioned Showcase";
    config.width                    = 1060;
    config.height                   = 700;
    config.window_mode              = WindowMode::Normal;
    config.vsync                    = false;
    config.target_fps               = 0;
    config.show_performance_overlay = true;
    config.clear_color              = Style::bg_dark;

    return runApp(std::make_shared<StackDemoApp>(), config);
}
