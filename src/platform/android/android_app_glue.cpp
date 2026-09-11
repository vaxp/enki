/// @file android_app_glue.cpp
/// @brief NativeActivity entry point and lifecycle bridge for enki on Android.
///
/// This file provides:
///   1. ANativeActivity_onCreate() — the NDK entry point called by the OS.
///   2. NativeActivity callbacks that forward lifecycle events to AndroidPlatformBackend.
///   3. A dedicated engine worker thread so that ANativeActivity_onCreate can return
///      immediately, allowing Android to create the native window and dispatch events.
///   4. The declaration of enki_android_main() that the APPLICATION defines.

#ifdef __ANDROID__

#include "enki/platform/android/android_platform.hpp"
#include "enki/platform/platform.hpp"
#include "enki/core/types.hpp"

#include <android/native_activity.h>
#include <android/configuration.h>
#include <android/log.h>

#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>

// ── User entry point (user's main() renamed via #define in app.hpp) ─────────
/// The developer writes int main(); app.hpp renames it to enki_user_main via
/// #define. Since it's a C++ function we declare it without extern "C".
int enki_user_main();

// ── Screen size accessor ─────────────────────────────────────────────────────
// Stores the ANativeWindow* that was ready before main() was called.
// enki::getScreenSize() reads from this to let main() query screen dimensions.
static ANativeWindow*   s_boot_window   = nullptr;
static ANativeActivity* s_boot_activity = nullptr;

namespace enki {
/// Returns the logical screen size in density-independent pixels (dp).
/// On Android this queries ANativeWindow and scales by the display density.
/// Always safe to call from main() before runApp().
Size getScreenSize() {
    if (s_boot_window) {
        float dpi = 1.0f;
        if (s_boot_activity && s_boot_activity->assetManager) {
            auto* cfg = AConfiguration_new();
            AConfiguration_fromAssetManager(cfg, s_boot_activity->assetManager);
            int32_t density = AConfiguration_getDensity(cfg);
            AConfiguration_delete(cfg);
            if (density > 0) {
                dpi = static_cast<float>(density) / 160.0f;
            }
        }
        if (dpi <= 0.0f) dpi = 1.0f;
        return {
            static_cast<float>(ANativeWindow_getWidth(s_boot_window)) / dpi,
            static_cast<float>(ANativeWindow_getHeight(s_boot_window)) / dpi
        };
    }
    return {0.0f, 0.0f};
}
} // namespace enki

/// @brief Framework-owned Android entry point called by NativeActivity.
/// Bridges NativeActivity lifecycle to the user's int main().
extern "C" int enki_android_main() {
    return enki_user_main();
}


// ════════════════════════════════════════════════════════════════
// Internal glue state — one per NativeActivity instance
// ════════════════════════════════════════════════════════════════

namespace {

using enki::android::EnkiAndroidGlue;

inline EnkiAndroidGlue* getGlue(ANativeActivity* activity) {
    if (!activity) return nullptr;
    return reinterpret_cast<EnkiAndroidGlue*>(activity->instance);
}

// ── NativeActivity Callbacks ─────────────────────────────────────

static void onStart(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onStart");
}

static void onResume(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onResume");
    if (auto* g = getGlue(activity); g && g->backend) {
        g->backend->onResume();
    }
}

static void onPause(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onPause");
    if (auto* g = getGlue(activity); g && g->backend) {
        g->backend->onPause();
    }
}

static void onStop(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onStop");
}

static void onDestroy(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onDestroy");
    auto* g = getGlue(activity);
    if (g) {
        if (g->backend) {
            g->backend->onDestroy();
        }
        {
            std::lock_guard<std::mutex> lock(g->mutex);
            g->destroyed = true;
            g->cv.notify_all();
        }
        if (g->app_thread.joinable() && g->app_thread.get_id() != std::this_thread::get_id()) {
            g->app_thread.join();
        }
        delete g;
        activity->instance = nullptr;
    }
}

static void onWindowFocusChanged(ANativeActivity* activity, int has_focus) {
    if (auto* g = getGlue(activity); g && g->backend) {
        g->backend->onWindowFocusChanged(has_focus != 0);
    }
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    ENKI_ALOG("NativeActivity::onNativeWindowCreated: %p", window);
    auto* g = getGlue(activity);
    if (g) {
        g->native_window = window;
        if (g->backend) {
            g->backend->onNativeWindowCreated(window);
        }
        {
            std::lock_guard<std::mutex> lock(g->mutex);
            g->window_created = true;
            g->cv.notify_all();
        }
    }
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    ENKI_ALOG("NativeActivity::onNativeWindowDestroyed: %p", window);
    auto* g = getGlue(activity);
    if (g) {
        g->native_window = nullptr;
        {
            std::lock_guard<std::mutex> lock(g->mutex);
            g->window_created = false;
        }
        if (g->backend) {
            g->backend->onNativeWindowDestroyed(window);
        }
    }
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    ENKI_ALOG("NativeActivity::onInputQueueCreated: %p", queue);
    auto* g = getGlue(activity);
    if (g) {
        g->input_queue = queue;
        if (g->backend) {
            g->backend->onInputQueueCreated(queue);
        }
    }
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    ENKI_ALOG("NativeActivity::onInputQueueDestroyed: %p", queue);
    auto* g = getGlue(activity);
    if (g) {
        g->input_queue = nullptr;
        if (g->backend) {
            g->backend->onInputQueueDestroyed(queue);
        }
    }
}

static void onConfigurationChanged(ANativeActivity* activity) {
    ENKI_ALOG("NativeActivity::onConfigurationChanged");
    if (auto* g = getGlue(activity); g && g->backend) {
        g->backend->updateOutputFromConfig();
    }
}

static void onLowMemory(ANativeActivity* /*activity*/) {
    ENKI_ALOGW("NativeActivity::onLowMemory");
}

void appThreadEntry(ANativeActivity* activity, EnkiAndroidGlue* glue) {
    ENKI_ALOG("appThreadEntry started — waiting for native window");

    // Wait until onNativeWindowCreated has fired
    {
        std::unique_lock<std::mutex> lock(glue->mutex);
        glue->cv.wait(lock, [glue]() {
            return glue->window_created || glue->destroyed;
        });
        if (glue->destroyed) {
            ENKI_ALOG("Activity destroyed before window created — exiting thread");
            return;
        }
    }

    ENKI_ALOG("Native window ready — registering activity and invoking user main");
    enki::Platform::setAndroidActivity(activity);

    // Make the native window available to enki::getScreenSize() before main() runs
    s_boot_window   = glue->native_window;
    s_boot_activity = activity;

    int ret = enki_android_main();
    ENKI_ALOG("User main() returned %d — finishing activity", ret);

    ANativeActivity_finish(activity);
}

} // anonymous namespace

// ════════════════════════════════════════════════════════════════
// ANativeActivity_onCreate — NDK entry point
// ════════════════════════════════════════════════════════════════

extern "C" JNIEXPORT void ANativeActivity_onCreate(
    ANativeActivity* activity,
    void*            savedState,
    size_t           savedStateSize)
{
    (void)savedState;
    (void)savedStateSize;

    ENKI_ALOG("ANativeActivity_onCreate — enki version 0.1");

    // Register all NativeActivity callbacks
    activity->callbacks->onStart                = onStart;
    activity->callbacks->onResume               = onResume;
    activity->callbacks->onPause                = onPause;
    activity->callbacks->onStop                 = onStop;
    activity->callbacks->onDestroy              = onDestroy;
    activity->callbacks->onWindowFocusChanged   = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated  = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed= onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated    = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed  = onInputQueueDestroyed;
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory            = onLowMemory;

    auto* glue = new EnkiAndroidGlue();
    glue->activity = activity;
    activity->instance = glue;

    // Start background engine thread
    glue->app_thread = std::thread(appThreadEntry, activity, glue);
}

#endif // __ANDROID__
