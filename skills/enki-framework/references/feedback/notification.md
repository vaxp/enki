# Notification System

> A multi-channel desktop notification architecture featuring floating transient push toasts, interactive action buttons, an unread count badge bell, and a slide-out Notification Center feed drawer.

- **Header File**: `#include "enki/widgets/notification.hpp"`
- **Overlay Widget**: `enki::NotificationOverlay` (wraps page content)
- **Bell Indicator**: `enki::NotificationBell`
- **Central Manager**: `enki::NotificationManager`
- **Item Descriptor**: `enki::NotificationItem`
- **Action Descriptor**: `enki::NotificationAction`
- **Enums**: `enki::NotificationSeverity`, `enki::NotificationCategory`

---

## Overview

Enki's `Notification` subsystem bridges the gap between transient alerts and persistent history. It operates through a unified `NotificationManager`:
1. **Floating Push Toasts**: When a notification is posted (`show_push_toast = true`), it floats over the top-right corner of the window with an auto-dismiss countdown timer.
2. **Notification Center Feed**: Notifications are concurrently logged into a persistent slide-out feed drawer where users can filter by category (`System`, `Security`, `Mentions`), review history, and mark items as read.
3. **Notification Bell**: Displays the unread count badge on top of a bell icon and toggles the drawer.

---

## C++ API Definition

### Enums & Action Descriptor
```cpp
namespace enki {

enum class NotificationSeverity {
    Info,       ///< Informational alert (Sky blue)
    Success,    ///< Successful completion (Emerald green)
    Warning,    ///< Cautionary notice (Amber yellow)
    Error,      ///< Failure or critical error (Crimson red)
    Security    ///< Authentication & security notice (Purple / Indigo)
};

enum class NotificationCategory {
    All,
    System,
    Security,
    Mentions,
    Updates
};

struct NotificationAction {
    std::string           id         = "";
    std::string           label      = "";
    bool                  is_primary = false;
    bool                  is_danger  = false;
    std::function<void()> on_click;

    NotificationAction(std::string id, std::string label,
                       bool primary = false, bool danger = false,
                       std::function<void()> cb = nullptr);
};

} // namespace enki
```

### Notification Item Descriptor
```cpp
namespace enki {

struct NotificationItem {
    static NotificationItem info(std::string id, std::string title, std::string message,
                                 NotificationCategory cat = NotificationCategory::System);

    static NotificationItem success(std::string id, std::string title, std::string message,
                                    NotificationCategory cat = NotificationCategory::System);

    static NotificationItem warning(std::string id, std::string title, std::string message,
                                    NotificationCategory cat = NotificationCategory::System);

    static NotificationItem error(std::string id, std::string title, std::string message,
                                  NotificationCategory cat = NotificationCategory::System);

    static NotificationItem security(std::string id, std::string title, std::string message);

    NotificationItem& addAction(NotificationAction act);
    NotificationItem& setSender(std::string sender);
    NotificationItem& setTime(std::string time);
    NotificationItem& setIcon(std::string icon);
};

} // namespace enki
```

### Manager & Declarative Structs
```cpp
namespace enki {

class NotificationManager {
public:
    void post(NotificationItem item, bool show_push_toast = true);
    void dismissPushToast(const std::string& id);
    void markAsRead(const std::string& id);
    void markAllAsRead();
    void remove(const std::string& id);
    void clearAll();

    [[nodiscard]] int getUnreadCount() const;
    void toggleCenter();
    void openCenter();
    void closeCenter();
};

struct NotificationOverlay {
    Key                                  key     = Key::none();
    WidgetPtr                            body    = nullptr;
    std::shared_ptr<NotificationManager> manager = nullptr;

    operator WidgetPtr() const;
};

struct NotificationBell {
    Key                                  key       = Key::none();
    std::shared_ptr<NotificationManager> manager   = nullptr;
    std::string                          bell_icon = "🔔";

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Code Examples (From `widgets_demo/notification_demo/main.cpp`)

### 1. Setting up the Notification Overlay & Bell
```cpp
#include "enki/widgets/notification.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class AppShellState : public State {
    std::shared_ptr<NotificationManager> notify_ = std::make_shared<NotificationManager>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        // Top App Bar with Bell Icon
        auto topBar = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .padding = EdgeInsets::all(16.0f),
            .children = {
                text("Dashboard Workspace", { .font_weight = FontWeight::Bold }),
                NotificationBell { .manager = notify_ } // Displays unread count badge
            }
        });

        auto pageBody = column({
            .children = {
                topBar,
                button(text("Deploy Cluster"), [this]() {
                    notify_->post(NotificationItem::success("dep_1", "Deployed", "Kubernetes cluster is live!")
                        .addAction(NotificationAction("view", "View Cluster", true, false, []{
                            std::cout << "Viewing cluster...\n";
                        }))
                    );
                })
            }
        });

        // Wrap page in NotificationOverlay
        return NotificationOverlay {
            .body = pageBody,
            .manager = notify_
        };
    }
};
```

### 2. Security Alert Notification with Danger Action
```cpp
auto alert = NotificationItem::security("sec_login", "Suspicious Login Detected", "Login from untrusted IP.")
    .setTime("Just now")
    .addAction(NotificationAction("block", "Block IP", false, true, []{
        // Block action
    }))
    .addAction(NotificationAction("allow", "Trust Session", true, false, []{
        // Approve action
    }));

notifyManager->post(alert, true);
```

---

## See Also
- [**Snackbar**](../Overlays/snackbar.md) — Simple transient toast notifications.
- [**LoadingOverlay**](./loading_overlay.md) — Modal busy screen with progress feedback.
