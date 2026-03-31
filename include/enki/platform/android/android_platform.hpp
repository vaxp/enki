#pragma once
/// @file android_platform.hpp
/// @brief Android (NDK NativeActivity) platform backend.
/// Internal backend used by Platform when compiled for __ANDROID__.
/// Mirrors the interface of Win32PlatformBackend and X11PlatformBackend.

#ifdef __ANDROID__

#include "enki/platform/platform.hpp"
#include "enki/platform/output.hpp"

#include <android/native_activity.h>
#include <android/input.h>
#include <android/looper.h>
#include <android/configuration.h>
#include <android/log.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#define ENKI_ANDROID_LOG_TAG "enki"
#define ENKI_ALOG(...)  __android_log_print(ANDROID_LOG_DEBUG,  ENKI_ANDROID_LOG_TAG, __VA_ARGS__)
#define ENKI_ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,  ENKI_ANDROID_LOG_TAG, __VA_ARGS__)
#define ENKI_ALOGW(...) __android_log_print(ANDROID_LOG_WARN,   ENKI_ANDROID_LOG_TAG, __VA_ARGS__)

namespace enki {
class Window;
} // namespace enki

namespace enki::android {

// ── Forward declarations ──────────────────────────────────────
class AndroidSurface;
class AndroidPlatformBackend;

/// Shared glue state stored in ANativeActivity::instance.
struct EnkiAndroidGlue {
    ANativeActivity*                      activity       = nullptr;
    AndroidPlatformBackend*               backend        = nullptr;
    ANativeWindow*                        native_window  = nullptr;
    AInputQueue*                          input_queue    = nullptr;
    std::thread                           app_thread;
    std::mutex                            mutex;
    std::condition_variable               cv;
    bool                                  window_created = false;
    bool                                  destroyed      = false;
};

// ────────────────────────────────────────────────────────────────
// AndroidPlatformBackend
// ────────────────────────────────────────────────────────────────

/// Android platform backend — owns EGL, input queue, and lifecycle hooks.
/// Created and owned by Platform::Impl when compiled for Android.
class AndroidPlatformBackend {
public:
    explicit AndroidPlatformBackend(Platform* owner, ANativeActivity* activity);
    ~AndroidPlatformBackend();

    // Non-copyable, non-movable
    AndroidPlatformBackend(const AndroidPlatformBackend&) = delete;
    AndroidPlatformBackend& operator=(const AndroidPlatformBackend&) = delete;

    bool init();
    void shutdown();

    /// Poll ALooper for pending events and dispatch them.
    /// @return false if the application has been asked to quit.
    bool pollEvents();

    // ── EGL accessors (used by AndroidSurface) ───────────────────
    [[nodiscard]] EGLDisplay getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig  getEGLConfig()  const { return egl_config_; }
    [[nodiscard]] EGLContext getEGLContext() const { return egl_context_; }

    // ── Activity / Looper ────────────────────────────────────────
    [[nodiscard]] ANativeActivity* getActivity() const { return activity_; }
    [[nodiscard]] ALooper*         getLooper()   const { return looper_; }
    [[nodiscard]] AConfiguration*  getConfig()   const { return config_; }
    [[nodiscard]] ANativeWindow*   getNativeWindow() const { return current_native_window_; }

    // ── Clipboard (via JNI) ──────────────────────────────────────
    void setClipboardData(const ClipboardData& data, ClipboardType type = ClipboardType::Clipboard);
    void setClipboardText(std::string_view text, ClipboardType type = ClipboardType::Clipboard);
    [[nodiscard]] std::string getClipboardText(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    // ── Drag & Drop — no-op on Android ──────────────────────────
    bool startDrag(const DragData& /*data*/, DragAction /*actions*/) { return false; }

    // ── Cursor — no-op on Android ────────────────────────────────
    void setCursor(SystemCursor /*cursor*/) {}

    // ── Output / Monitor ─────────────────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    void updateOutputFromConfig();

    // ── Foreign Toplevel — not applicable on Android ─────────────
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const { return {}; }
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const { return nullptr; }

    // ── Lifecycle callbacks (called by android_app_glue) ─────────

    /// Called when the ANativeWindow is ready for rendering.
    /// AndroidSurface registers itself to receive this notification.
    void onNativeWindowCreated(ANativeWindow* window);

    /// Called when the ANativeWindow is about to be destroyed.
    void onNativeWindowDestroyed(ANativeWindow* window);

    /// Called when focus changes.
    void onWindowFocusChanged(bool has_focus);

    /// Called when the app should pause (e.g. home button pressed).
    void onPause();

    /// Called when the app resumes.
    void onResume();

    /// Called when the process should terminate.
    void onDestroy();

    /// Attach an input queue for event processing.
    void onInputQueueCreated(AInputQueue* queue);

    /// Detach the input queue.
    void onInputQueueDestroyed(AInputQueue* queue);

    // ── Surface registration ─────────────────────────────────────
    void registerSurface(AndroidSurface* surface);
    void unregisterSurface(AndroidSurface* surface);

    [[nodiscard]] Platform* getOwner() const { return owner_; }

    // ── Display metrics ──────────────────────────────────────────
    [[nodiscard]] float getDpiScale() const { return dpi_scale_; }

private:
    Platform*       owner_    = nullptr;
    ANativeActivity* activity_ = nullptr;
    ALooper*         looper_   = nullptr;
    AConfiguration*  config_   = nullptr;
    AInputQueue*     input_queue_ = nullptr;
    ANativeWindow*   current_native_window_ = nullptr;

    // EGL shared context
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLConfig  egl_config_  = nullptr;
    EGLContext egl_context_ = EGL_NO_CONTEXT;

    // Display info
    float dpi_scale_ = 1.0f;
    int   screen_width_  = 0;
    int   screen_height_ = 0;

    // Clipboard fallback buffer
    mutable ClipboardData clipboard_buffer_;

    // Registered surfaces (usually just one)
    std::unordered_set<AndroidSurface*> surfaces_;

    std::atomic<bool> quit_requested_{ false };
    bool paused_ = false;
    bool activated_ = false;

    // Output representing the device screen
    class AndroidOutput;
    std::shared_ptr<AndroidOutput> primary_output_;

    // ── EGL initialisation ───────────────────────────────────────
    bool initEGL();
    void destroyEGL();

    // ── Input processing ─────────────────────────────────────────
    void processInputQueue();
    void handleInputEvent(AInputEvent* event);
    void handleTouchEvent(AInputEvent* event);
    void handleKeyEvent(AInputEvent* event);

    // ── JNI helpers ──────────────────────────────────────────────
    std::string jniGetClipboardText() const;
    void        jniSetClipboardText(std::string_view text) const;

    // ── ALooper pipe for wakeup ──────────────────────────────────
    int pipe_read_fd_  = -1;
    int pipe_write_fd_ = -1;
    void createWakeupPipe();
    void destroyWakeupPipe();
    static int looperCallback(int fd, int events, void* data);
};

} // namespace enki::android

#endif // __ANDROID__
