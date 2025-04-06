#pragma once
/// @file native_popup.hpp
/// @brief Native Compositor-level Popup surface manager for Desktop Shells.

#include "enki/core/types.hpp"
#include "enki/shell/shell_types.hpp"
#include "enki/shell/surface_host.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/widget.hpp"

#include <memory>
#include <functional>

namespace enki {

/// @brief Represents an active native popup surface spawned outside a panel or window.
class NativePopup : public std::enable_shared_from_this<NativePopup> {
public:
    /// Show a native compositor popup anchored to a screen rect or widget.
    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        const PopupOptions& options,
        std::function<WidgetPtr(BuildContext&)> builder
    );

    /// Show a native compositor popup with access to the NativePopup handle in the builder.
    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        const PopupOptions& options,
        std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> builder
    );

    ~NativePopup();

    /// Close and destroy the native popup surface.
    void close();

    /// Check if the popup is currently visible.
    [[nodiscard]] bool isOpen() const { return host_ != nullptr; }

    /// Access the underlying SurfaceHost.
    [[nodiscard]] SurfaceHost* surfaceHost() const { return host_; }

private:
    NativePopup(const PopupOptions& options);

    PopupOptions options_;
    SurfaceHost* host_ = nullptr;
};

} // namespace enki
