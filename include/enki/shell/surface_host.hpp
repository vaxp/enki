#pragma once
/// @file surface_host.hpp
/// @brief Per-surface element tree and rendering host for multi-surface desktop shell.

#include "enki/core/types.hpp"
#include "enki/platform/window.hpp"
#include "enki/platform/layer_surface.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkSurface.h>
#include <include/gpu/GrDirectContext.h>

#include <memory>
#include <vector>
#include <unordered_set>

namespace enki {

/// @brief Hosts a single native surface (Window or LayerSurface) along with its
/// independent Element and RenderObject widget tree.
class SurfaceHost {
public:
    SurfaceHost(std::unique_ptr<Window> window, WidgetPtr root_widget);
    SurfaceHost(std::unique_ptr<LayerSurface> layer_surface, WidgetPtr root_widget);
    ~SurfaceHost();

    // Non-copyable, movable
    SurfaceHost(const SurfaceHost&) = delete;
    SurfaceHost& operator=(const SurfaceHost&) = delete;
    SurfaceHost(SurfaceHost&&) noexcept;
    SurfaceHost& operator=(SurfaceHost&&) noexcept;

    /// Update or replace the root widget of this surface
    void setRootWidget(WidgetPtr root);

    /// Rebuild all dirty elements in this surface's tree
    void rebuild();

    /// Perform layout on the render tree for this surface
    void layout();

    /// Paint the render tree into the Skia surface
    void paint(GrDirectContext* gr_ctx, Color clear_color = 0x00000000);

    /// Present the rendered frame to the compositor
    void swapBuffers();

    /// Make this surface's EGL context/surface current
    void makeCurrent();

    /// Pointer input dispatch
    void handlePointerDown(float x, float y, MouseButton btn);
    void handlePointerUp(float x, float y, MouseButton btn);
    void handlePointerMove(float x, float y);
    void handleScroll(float dx, float dy);

    /// Check if this surface needs a rebuild or repaint
    [[nodiscard]] bool needsRedraw() const;

    /// Native handles and properties
    [[nodiscard]] void* getNativeHandle() const;
    [[nodiscard]] Size  getSize() const;
    [[nodiscard]] float getDpiScale() const;
    [[nodiscard]] bool  isLayerSurface() const { return layer_surface_ != nullptr; }

    LayerSurface* getLayerSurface() const { return layer_surface_.get(); }
    Window*       getWindow() const { return window_.get(); }
    Element*      getRootElement() const { return root_element_.get(); }

    /// Auto-dismiss status for popups
    void setAutoDismiss(bool dismiss) { auto_dismiss_ = dismiss; }
    [[nodiscard]] bool isAutoDismiss() const { return auto_dismiss_; }

    /// Signals
    Signal<>& onClose() { return on_close_; }

private:
    void initTree(WidgetPtr root);
    void recreateSkSurface(GrDirectContext* gr_ctx, int w, int h);

    std::unique_ptr<Window>       window_;
    std::unique_ptr<LayerSurface> layer_surface_;

    WidgetPtr                   root_widget_;
    std::unique_ptr<BuildOwner> build_owner_;
    std::unique_ptr<Element>    root_element_;

    sk_sp<SkSurface> cached_surface_;
    int              cached_w_ = 0;
    int              cached_h_ = 0;

    // Pointer state
    std::unordered_set<RenderObject*> hovered_targets_;
    std::vector<RenderObject*>        active_pointer_targets_;
    bool                              is_pointer_down_ = false;
    MouseButton                       active_button_   = MouseButton::None;
    float                             last_pointer_x_  = 0.0f;
    float                             last_pointer_y_  = 0.0f;
    bool                              auto_dismiss_    = true;

    Signal<> on_close_;
};

} // namespace enki
