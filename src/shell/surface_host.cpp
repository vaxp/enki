/// @file surface_host.cpp
/// @brief Per-surface element tree and rendering host implementation.

#include "enki/shell/surface_host.hpp"

#include <include/core/SkCanvas.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <GL/gl.h>

#include <iostream>
#include <algorithm>

namespace enki {

// Forward declaration of internal canvas wrapper factory
std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas);

SurfaceHost::SurfaceHost(std::unique_ptr<Window> window, WidgetPtr root_widget)
    : window_(std::move(window)), root_widget_(std::move(root_widget)) {
    if (window_) {
        window_->onClose().connect([this] { on_close_.emit(); });
    }
    initTree(root_widget_);
}

SurfaceHost::SurfaceHost(std::unique_ptr<LayerSurface> layer_surface, WidgetPtr root_widget)
    : layer_surface_(std::move(layer_surface)), root_widget_(std::move(root_widget)) {
    if (layer_surface_) {
        layer_surface_->onClose().connect([this] { on_close_.emit(); });
    }
    initTree(root_widget_);
}

SurfaceHost::~SurfaceHost() {
    if (root_element_) {
        root_element_->unmount();
    }
    cached_surface_.reset();
    root_element_.reset();
    build_owner_.reset();
}

SurfaceHost::SurfaceHost(SurfaceHost&&) noexcept = default;
SurfaceHost& SurfaceHost::operator=(SurfaceHost&&) noexcept = default;

void SurfaceHost::initTree(WidgetPtr root) {
    if (!root) return;
    build_owner_ = std::make_unique<BuildOwner>();
    root_element_ = root->createElement();
    if (root_element_) {
        root_element_->setOwner(build_owner_.get());
        root_element_->mount(nullptr, 0);
        build_owner_->buildScope(root_element_.get());
    }
}

void SurfaceHost::setRootWidget(WidgetPtr root) {
    root_widget_ = std::move(root);
    if (!root_element_) {
        initTree(root_widget_);
    } else if (root_widget_) {
        root_element_->update(root_widget_);
    }
}

void SurfaceHost::rebuild() {
    if (build_owner_ && root_element_) {
        build_owner_->buildScope(root_element_.get());
    }
}

void SurfaceHost::layout() {
    if (!root_element_) return;
    RenderObject* ro = root_element_->findRenderObject();
    if (!ro) return;

    Size sz = getSize();
    if (sz.width <= 0 || sz.height <= 0) return;

    if (ro->needsLayout()) {
        ro->layout(sz.width, sz.height);
    }
}

void SurfaceHost::makeCurrent() {
    if (window_) {
        window_->makeCurrent();
    } else if (layer_surface_) {
        layer_surface_->makeCurrent();
    }
}

void SurfaceHost::recreateSkSurface(GrDirectContext* gr_ctx, int w, int h) {
    if (w <= 0 || h <= 0 || !gr_ctx) return;

    makeCurrent();

    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    GLint samples = 0;
    GLint stencil = 8;

    GrGLFramebufferInfo fb_info;
    fb_info.fFBOID = static_cast<GrGLuint>(fbo);
    fb_info.fFormat = 0x8058; // GL_RGBA8

    auto target = GrBackendRenderTarget(
        w, h,
        samples,
        stencil,
        fb_info
    );

    cached_surface_ = SkSurface::MakeFromBackendRenderTarget(
        gr_ctx,
        target,
        kBottomLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType,
        nullptr,
        nullptr
    );

    cached_w_ = w;
    cached_h_ = h;
}

void SurfaceHost::paint(GrDirectContext* gr_ctx, Color clear_color) {
    if (!root_element_ || !gr_ctx) return;
    RenderObject* ro = root_element_->findRenderObject();
    if (!ro) return;

    Size sz = getSize();
    int w = static_cast<int>(sz.width);
    int h = static_cast<int>(sz.height);
    if (w <= 0 || h <= 0) return;

    makeCurrent();
    gr_ctx->resetContext();

    if (!cached_surface_ || cached_w_ != w || cached_h_ != h) {
        recreateSkSurface(gr_ctx, w, h);
    }

    if (!cached_surface_) return;

    SkCanvas* sk_canvas = cached_surface_->getCanvas();
    if (!sk_canvas) return;

    // Clear background
    const Color cc = clear_color;
    sk_canvas->clear(SkColorSetARGB(
        (cc >> 24) & 0xFF,
        (cc >> 16) & 0xFF,
        (cc >>  8) & 0xFF,
        (cc >>  0) & 0xFF
    ));

    auto canvas = createCanvasWrapper(sk_canvas);
    PaintContext pctx{*canvas, Point{0.0f, 0.0f}, Rect{0.0f, 0.0f, sz.width, sz.height}, 1.0f};
    ro->paint(pctx);

    gr_ctx->flushAndSubmit();
}

void SurfaceHost::swapBuffers() {
    if (window_) {
        window_->swapBuffers();
    } else if (layer_surface_) {
        layer_surface_->swapBuffers();
    }
}

bool SurfaceHost::needsRedraw() const {
    if (!build_owner_ || !root_element_) return false;
    RenderObject* ro = root_element_->findRenderObject();
    return build_owner_->hasDirtyElements() || (ro && ro->needsLayout());
}

void* SurfaceHost::getNativeHandle() const {
    if (window_) return window_->getNativeHandle();
    if (layer_surface_) return layer_surface_->getNativeHandle();
    return nullptr;
}

Size SurfaceHost::getSize() const {
    if (window_) return window_->getSize();
    if (layer_surface_) return layer_surface_->getSize();
    return {0.0f, 0.0f};
}

float SurfaceHost::getDpiScale() const {
    if (window_) return window_->getDpiScale();
    if (layer_surface_) return layer_surface_->getDpiScale();
    return 1.0f;
}

// ── Pointer Event Dispatching ────────────────────────────────────

void SurfaceHost::handlePointerDown(float x, float y, MouseButton btn) {
    if (!root_element_) return;
    RenderObject* ro = root_element_->findRenderObject();
    if (!ro) return;

    is_pointer_down_ = true;
    active_button_   = btn;
    last_pointer_x_  = x;
    last_pointer_y_  = y;
    active_pointer_targets_.clear();

    HitTestResult result;
    ro->hitTest(result, {x, y});

    for (const auto& entry : result.path()) {
        if (entry.target && RenderObject::isAlive(entry.target)) {
            active_pointer_targets_.push_back(entry.target);
            PointerEvent ev{
                .type          = PointerEvent::Down,
                .position      = {x, y},
                .localPosition = entry.localPosition,
                .button        = btn
            };
            entry.target->handlePointerDown(ev);
        }
    }
}

void SurfaceHost::handlePointerUp(float x, float y, MouseButton btn) {
    is_pointer_down_ = false;
    last_pointer_x_  = x;
    last_pointer_y_  = y;

    if (!active_pointer_targets_.empty()) {
        for (RenderObject* ro : active_pointer_targets_) {
            if (RenderObject::isAlive(ro)) {
                Rect bounds = ro->globalBounds();
                Point localPos = {x - bounds.x, y - bounds.y};
                PointerEvent ev{
                    .type          = PointerEvent::Up,
                    .position      = {x, y},
                    .localPosition = localPos,
                    .button        = btn
                };
                ro->handlePointerUp(ev);
            }
        }
        active_pointer_targets_.clear();
    } else {
        if (!root_element_) return;
        RenderObject* ro = root_element_->findRenderObject();
        if (!ro) return;

        HitTestResult result;
        ro->hitTest(result, {x, y});

        for (const auto& entry : result.path()) {
            if (entry.target && RenderObject::isAlive(entry.target)) {
                PointerEvent ev{
                    .type          = PointerEvent::Up,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = btn
                };
                entry.target->handlePointerUp(ev);
            }
        }
    }
}

void SurfaceHost::handlePointerMove(float x, float y) {
    last_pointer_x_ = x;
    last_pointer_y_ = y;

    if (!root_element_) return;
    RenderObject* ro = root_element_->findRenderObject();
    if (!ro) return;

    if (is_pointer_down_) {
        for (RenderObject* target : active_pointer_targets_) {
            if (RenderObject::isAlive(target)) {
                Rect bounds = target->globalBounds();
                Point localPos = {x - bounds.x, y - bounds.y};
                PointerEvent ev{
                    .type          = PointerEvent::Move,
                    .position      = {x, y},
                    .localPosition = localPos,
                    .button        = active_button_
                };
                target->handlePointerMove(ev);
            }
        }
    }

    HitTestResult result;
    ro->hitTest(result, {x, y});

    std::unordered_set<RenderObject*> current_hovered;

    for (const auto& entry : result.path()) {
        if (entry.target && RenderObject::isAlive(entry.target)) {
            current_hovered.insert(entry.target);

            if (!is_pointer_down_) {
                PointerEvent ev{
                    .type          = PointerEvent::Move,
                    .position      = {x, y},
                    .localPosition = entry.localPosition,
                    .button        = MouseButton::None
                };
                entry.target->handlePointerMove(ev);
            }
        }
    }

    // Exited targets
    for (RenderObject* target : hovered_targets_) {
        if (RenderObject::isAlive(target) && current_hovered.find(target) == current_hovered.end()) {
            target->handlePointerExit({});
        }
    }

    // Entered targets
    for (RenderObject* target : current_hovered) {
        if (RenderObject::isAlive(target) && hovered_targets_.find(target) == hovered_targets_.end()) {
            target->handlePointerEnter({});
        }
    }

    hovered_targets_ = std::move(current_hovered);
}

void SurfaceHost::handleScroll(float dx, float dy) {
    if (!root_element_) return;
    RenderObject* ro = root_element_->findRenderObject();
    if (!ro) return;

    HitTestResult result;
    ro->hitTest(result, {last_pointer_x_, last_pointer_y_});

    for (const auto& entry : result.path()) {
        if (entry.target && RenderObject::isAlive(entry.target) && entry.target->handlesScroll()) {
            entry.target->handlePointerScroll(dx, dy);
            break;
        }
    }
}

} // namespace enki
