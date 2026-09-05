/// @file hero.cpp
/// @brief Shared Element Hero Transition System implementation.

#include "enki/widgets/hero.hpp"
#include "enki/rendering/canvas.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// HeroRegistry
// ════════════════════════════════════════════════════════════════

void HeroRegistry::registerHero(const std::string& tag, RenderHero* ro, WidgetPtr w) {
    if (tag.empty()) return;
    HeroRecord rec;
    rec.tag = tag;
    rec.render_object = ro;
    rec.widget = std::move(w);
    heroes_[tag] = std::move(rec);
}

void HeroRegistry::unregisterHero(const std::string& tag, RenderHero* ro) {
    auto it = heroes_.find(tag);
    if (it != heroes_.end() && it->second.render_object == ro) {
        heroes_.erase(it);
    }
}

void HeroRegistry::updateBounds(const std::string& tag, RenderHero* ro, const Rect& bounds) {
    auto it = heroes_.find(tag);
    if (it != heroes_.end() && it->second.render_object == ro) {
        it->second.last_global_bounds = bounds;
    }
}

const HeroRecord* HeroRegistry::findHero(const std::string& tag) const {
    auto it = heroes_.find(tag);
    if (it != heroes_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool HeroRegistry::hasHero(const std::string& tag) const {
    return heroes_.find(tag) != heroes_.end();
}

void HeroRegistry::clear() {
    heroes_.clear();
}

// ════════════════════════════════════════════════════════════════
// RenderHero
// ════════════════════════════════════════════════════════════════

RenderHero::~RenderHero() {
    HeroRegistry::instance().unregisterHero(tag, this);
}

void RenderHero::paint(PaintContext& ctx) {
    // Record current global position in registry
    Rect gb = globalBounds();
    HeroRegistry::instance().updateBounds(tag, this, gb);

    if (is_placeholder) {
        // Child is currently in flight, hide local copy
        return;
    }

    // Paint child normally
    for (auto* child : children()) {
        if (child) {
            child->paint(ctx);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// HeroWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> HeroWidget::createRenderObject(BuildContext&) {
    auto rh = std::make_unique<RenderHero>(tag);
    HeroRegistry::instance().registerHero(tag, rh.get(), child);
    return rh;
}

void HeroWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& rh = static_cast<RenderHero&>(renderObject);
    if (rh.tag != tag) {
        HeroRegistry::instance().unregisterHero(rh.tag, &rh);
        rh.tag = tag;
        HeroRegistry::instance().registerHero(tag, &rh, child);
    }
    rh.markNeedsPaint();
}

// ════════════════════════════════════════════════════════════════
// RenderHeroFlight
// ════════════════════════════════════════════════════════════════

void RenderHeroFlight::paint(PaintContext& ctx) {
    if (children().empty()) return;

    float t = std::clamp(progress, 0.0f, 1.0f);

    float cur_x = start_rect.x + (end_rect.x - start_rect.x) * t;
    float cur_y = start_rect.y + (end_rect.y - start_rect.y) * t;
    float cur_w = start_rect.width + (end_rect.width - start_rect.width) * t;
    float cur_h = start_rect.height + (end_rect.height - start_rect.height) * t;

    auto* child = static_cast<RenderBox*>(children()[0]);
    Size child_size = child->size();
    float cw = (child_size.width > 0.0f) ? child_size.width : cur_w;
    float ch = (child_size.height > 0.0f) ? child_size.height : cur_h;

    float sx = (cw > 0.0f) ? cur_w / cw : 1.0f;
    float sy = (ch > 0.0f) ? cur_h / ch : 1.0f;

    ctx.canvas.save();
    ctx.canvas.translate(cur_x, cur_y);
    ctx.canvas.scale(sx, sy);

    PaintContext flight_ctx{ctx.canvas, {0.0f, 0.0f}, ctx.clip_rect, ctx.opacity};
    child->paint(flight_ctx);

    ctx.canvas.restore();
}

} // namespace enki
