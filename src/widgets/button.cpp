#include "enki/widgets/button.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/effects/SkRuntimeEffect.h>

#include <chrono>
#include <vector>
#include <iostream>

namespace enki {

inline double getCurrentTimeSeconds() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0;
}

// ════════════════════════════════════════════════════════════════
// Ripple State
// ════════════════════════════════════════════════════════════════

struct RippleState {
    Point center;
    float radius = 0.0f;
    float alpha = 1.0f;
    bool active = true;
};

struct RippleAnim {
    RippleState state;
    float target_radius = 0.0f;
};

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Background (Shaders, Ripples)
// ════════════════════════════════════════════════════════════════

class RenderButtonBackground : public RenderBox {
public:
    ButtonProps options;
    bool disabled;
    float hover_progress;
    float press_progress;
    std::vector<RippleState> ripples;
    double start_time;
    Point last_mouse_pos;

    sk_sp<SkRuntimeEffect> effect;

    RenderButtonBackground(const ButtonProps& opt, bool disabled, float hover, float press, 
                           const std::vector<RippleState>& rips, double st, Point mouse)
        : options(opt), disabled(disabled), hover_progress(hover), press_progress(press),
          ripples(rips), start_time(st), last_mouse_pos(mouse) {
        
        if (!options.custom_shader.empty()) {
            auto [eff, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (!eff) {
                std::cerr << "SkSL Shader Compile Error: " << err.c_str() << "\n";
            } else {
                effect = eff;
            }
        }
    }

    // Helper for color interpolation
    Color interpolateColor(Color a, Color b, float t) {
        uint8_t aA = (a >> 24) & 0xFF;
        uint8_t aR = (a >> 16) & 0xFF;
        uint8_t aG = (a >> 8) & 0xFF;
        uint8_t aB = a & 0xFF;

        uint8_t bA = (b >> 24) & 0xFF;
        uint8_t bR = (b >> 16) & 0xFF;
        uint8_t bG = (b >> 8) & 0xFF;
        uint8_t bB = b & 0xFF;

        uint8_t rA = aA + (bA - aA) * t;
        uint8_t rR = aR + (bR - aR) * t;
        uint8_t rG = aG + (bG - aG) * t;
        uint8_t rB = aB + (bB - aB) * t;

        return (rA << 24) | (rR << 16) | (rG << 8) | rB;
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        // Apply scale transform for press animation
        float scale = 1.0f - (0.02f * press_progress);
        
        if (scale < 1.0f) {
            canvas->save();
            float cx = ctx.offset.x + size_.width / 2.0f;
            float cy = ctx.offset.y + size_.height / 2.0f;
            canvas->translate(cx, cy);
            canvas->scale(scale, scale);
            canvas->translate(-cx, -cy);
        }

        SkRect rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(rect, options.border_radius, options.border_radius);

        // 1. Shadow
        if (options.shadow_blur > 0.0f && !disabled) {
            SkPaint shadow_paint;
            shadow_paint.setColor(options.shadow_color);
            shadow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.shadow_blur * 0.5f));
            canvas->save();
            canvas->translate(0, options.shadow_offset_dy);
            canvas->drawRRect(rrect, shadow_paint);
            canvas->restore();
        }

        // 2. Background (Shader or Color)
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        
        if (effect && !disabled) {
            double current_time = getCurrentTimeSeconds() - start_time;
            struct Uniforms {
                float time;
                float resolution[2];
            } uniforms = {
                (float)current_time,
                {size_.width, size_.height}
            };
            
            sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
            bg_paint.setShader(effect->makeShader(uniform_data, nullptr, 0));
            canvas->drawRRect(rrect, bg_paint);
        } else {
            Color current_bg;
            if (disabled) {
                current_bg = options.disabled_color;
            } else if (press_progress > 0.0f) {
                Color base = interpolateColor(options.normal_color, options.hover_color, hover_progress);
                current_bg = interpolateColor(base, options.pressed_color, press_progress);
            } else {
                current_bg = interpolateColor(options.normal_color, options.hover_color, hover_progress);
            }
            bg_paint.setColor(current_bg);
            canvas->drawRRect(rrect, bg_paint);
        }

        // 3. Ripples
        if (!ripples.empty()) {
            canvas->save();
            canvas->clipRRect(rrect, true);
            SkPaint ripple_paint;
            ripple_paint.setAntiAlias(true);
            for (const auto& rip : ripples) {
                uint32_t base_color = options.ripple_color;
                uint8_t a = ((base_color >> 24) & 0xFF) * rip.alpha;
                uint32_t final_color = (a << 24) | (base_color & 0x00FFFFFF);
                ripple_paint.setColor(final_color);
                
                // Point is in widget local space, convert to global space for canvas
                canvas->drawCircle(ctx.offset.x + rip.center.x, ctx.offset.y + rip.center.y, rip.radius, ripple_paint);
            }
            canvas->restore();
        }

        // 4. Paint Children
        if (!children().empty()) {
            RenderBox* child = static_cast<RenderBox*>(children()[0]);
            PaintContext child_ctx = ctx.withOffset(child->offset());
            child->paint(child_ctx);
        }

        if (scale < 1.0f) {
            canvas->restore();
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget for Background
// ════════════════════════════════════════════════════════════════

class ButtonBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    ButtonProps options;
    bool disabled;
    float hover_progress;
    float press_progress;
    std::vector<RippleState> ripples;
    double start_time;
    Point last_mouse_pos;

    ButtonBackgroundWidget(ButtonProps opt, bool disabled, float hover, float press, 
                           std::vector<RippleState> rips, double st, Point mouse, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)), disabled(disabled), hover_progress(hover), press_progress(press),
          ripples(std::move(rips)), start_time(st), last_mouse_pos(mouse) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderButtonBackground>(options, disabled, hover_progress, press_progress, ripples, start_time, last_mouse_pos);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderButtonBackground*>(&renderObject)) {
            rb->options = options;
            rb->disabled = disabled;
            rb->hover_progress = hover_progress;
            rb->press_progress = press_progress;
            rb->ripples = ripples;
            rb->start_time = start_time;
            rb->last_mouse_pos = last_mouse_pos;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "ButtonBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Button State
// ════════════════════════════════════════════════════════════════

class ButtonState : public State {
    AnimationController hover_anim;
    AnimationController press_anim;
    std::unique_ptr<Ticker> shader_ticker;

    std::vector<RippleAnim> ripples;
    double start_time = 0.0;
    Point last_mouse_pos = {0, 0};

public:
    void initState() override {
        State::initState();
        
        hover_anim.setDuration(std::chrono::milliseconds(200));
        hover_anim.addListener([this] { setState([]{}); });

        press_anim.setDuration(std::chrono::milliseconds(250));
        press_anim.addListener([this] { setState([]{}); });

        auto* btn = static_cast<const Button*>(this->widget());
        if (!btn->options.custom_shader.empty()) {
            start_time = getCurrentTimeSeconds();
            shader_ticker = createTicker([this]() {
                updateRipples();
                setState([]{});
            });
            shader_ticker->start();
        } else {
            shader_ticker = createTicker([this]() {
                updateRipples();
                if (ripples.empty()) shader_ticker->stop();
                setState([]{});
            });
        }
    }

    void dispose() override {
        hover_anim.stop();
        press_anim.stop();
        if (shader_ticker) shader_ticker->stop();
        State::dispose();
    }

    void updateRipples() {
        for (auto it = ripples.begin(); it != ripples.end();) {
            it->state.radius += (it->target_radius - it->state.radius) * 0.08f;
            if (it->state.radius > it->target_radius * 0.95f) {
                it->state.alpha -= 0.03f;
            }
            if (it->state.alpha <= 0.0f) {
                it = ripples.erase(it);
            } else {
                ++it;
            }
        }
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* btn = static_cast<const Button*>(this->widget());

        // Extract raw RippleStates to pass to the render object
        std::vector<RippleState> rip_states;
        rip_states.reserve(ripples.size());
        for (const auto& r : ripples) rip_states.push_back(r.state);

        // Build the layout inner component using Container for padding & alignment
        auto inner_container = container(btn->child);
        inner_container->padding(btn->options.padding)
                       .align(Alignment::Center)
                       .minWidth(StyleValue::point(btn->options.min_width))
                       .minHeight(StyleValue::point(btn->options.min_height));

        // Wrap with our custom Background renderer
        auto background = std::make_shared<ButtonBackgroundWidget>(
            btn->options, btn->disabled, hover_anim.value(), press_anim.value(),
            rip_states, start_time, last_mouse_pos, inner_container
        );

        // Wrap with GestureDetector
        auto gesture = gestureDetector(background);
        gesture->onHoverEnter([this, btn](const PointerEvent&) { if (!btn->disabled) hover_anim.forward(); });
        gesture->onHoverExit([this, btn](const PointerEvent&) { if (!btn->disabled) hover_anim.reverse(); });
        gesture->onHoverMove([this](const PointerEvent& e) { last_mouse_pos = e.localPosition; });
        
        gesture->onTapDown([this, btn](const TapDownDetails& e) {
            if (btn->disabled) return;
            press_anim.forward();
            last_mouse_pos = e.local_position;
            
            if (btn->options.enable_ripple) {
                RippleAnim r;
                r.state.center = e.local_position;
                r.target_radius = std::max(btn->options.min_width, btn->options.min_height) * 1.5f; // approximate max distance
                ripples.push_back(r);
                if (shader_ticker && !shader_ticker->isActive()) shader_ticker->start();
            }
        });
        
        gesture->onTapUp([this, btn](const TapUpDetails&) {
            if (btn->disabled) return;
            press_anim.reverse();
            if (btn->on_pressed) btn->on_pressed();
        });
        
        gesture->onTapCancel([this, btn]() {
            if (!btn->disabled) press_anim.reverse();
        });

        return gesture;
    }
};

std::unique_ptr<State> Button::createState() {
    return std::make_unique<ButtonState>();
}

} // namespace enki
