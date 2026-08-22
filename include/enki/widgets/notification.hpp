#pragma once
/// @file notification.hpp
/// @brief Advanced Notification System for ENKI Framework (Category 8. Feedback).
/// Features both Floating Push Toast Banners and an In-App Notification Center Feed Drawer,
/// multi-channel categorization, unread count badge management, and interactive actions.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>

namespace enki {

/// Notification severity level
enum class NotificationSeverity {
    Info,           ///< Informational (Sky blue)
    Success,        ///< Successful operation (Emerald green)
    Warning,        ///< Precautionary notice (Amber yellow)
    Error,          ///< Failure or critical error (Crimson red)
    Security        ///< Security & auth alerts (Purple / Indigo)
};

/// Category / Channel for notification filtering
enum class NotificationCategory {
    All,
    System,
    Security,
    Mentions,
    Updates
};

/// ════════════════════════════════════════════════════════════════
/// Notification Action Button
/// ════════════════════════════════════════════════════════════════

struct NotificationAction {
    std::string id = "";
    std::string label = "";
    bool is_primary = false;
    bool is_danger = false;
    std::function<void()> on_click;

    NotificationAction() = default;
    NotificationAction(std::string id_, std::string label_, bool primary = false,
                       bool danger = false, std::function<void()> cb = nullptr)
        : id(std::move(id_)), label(std::move(label_)), is_primary(primary),
          is_danger(danger), on_click(std::move(cb)) {}
};

/// ════════════════════════════════════════════════════════════════
/// Notification Item Model
/// ════════════════════════════════════════════════════════════════

struct NotificationItem {
    std::string id = "";
    std::string title = "";
    std::string message = "";
    std::string timestamp_str = "Just now";
    std::string icon = "ℹ️";
    std::string sender_name = "";

    NotificationSeverity severity = NotificationSeverity::Info;
    NotificationCategory category = NotificationCategory::System;

    bool is_read = false;
    bool is_pinned = false;
    int duration_ms = 5000; ///< Auto-dismiss timer for floating push toast (0 = persistent)

    std::vector<NotificationAction> actions;

    NotificationItem() = default;

    static NotificationItem info(std::string id, std::string title, std::string message,
                                 NotificationCategory cat = NotificationCategory::System) {
        NotificationItem n;
        n.id = std::move(id); n.title = std::move(title); n.message = std::move(message);
        n.severity = NotificationSeverity::Info; n.category = cat; n.icon = "ℹ️";
        return n;
    }

    static NotificationItem success(std::string id, std::string title, std::string message,
                                    NotificationCategory cat = NotificationCategory::System) {
        NotificationItem n;
        n.id = std::move(id); n.title = std::move(title); n.message = std::move(message);
        n.severity = NotificationSeverity::Success; n.category = cat; n.icon = "✅";
        return n;
    }

    static NotificationItem warning(std::string id, std::string title, std::string message,
                                    NotificationCategory cat = NotificationCategory::System) {
        NotificationItem n;
        n.id = std::move(id); n.title = std::move(title); n.message = std::move(message);
        n.severity = NotificationSeverity::Warning; n.category = cat; n.icon = "⚠️";
        return n;
    }

    static NotificationItem error(std::string id, std::string title, std::string message,
                                  NotificationCategory cat = NotificationCategory::System) {
        NotificationItem n;
        n.id = std::move(id); n.title = std::move(title); n.message = std::move(message);
        n.severity = NotificationSeverity::Error; n.category = cat; n.icon = "❌";
        return n;
    }

    static NotificationItem security(std::string id, std::string title, std::string message) {
        NotificationItem n;
        n.id = std::move(id); n.title = std::move(title); n.message = std::move(message);
        n.severity = NotificationSeverity::Security; n.category = NotificationCategory::Security; n.icon = "🛡️";
        return n;
    }

    NotificationItem& addAction(NotificationAction act) {
        actions.push_back(std::move(act));
        return *this;
    }

    NotificationItem& setSender(std::string sender) {
        sender_name = std::move(sender);
        return *this;
    }

    NotificationItem& setTime(std::string time) {
        timestamp_str = std::move(time);
        return *this;
    }

    NotificationItem& setIcon(std::string ic) {
        icon = std::move(ic);
        return *this;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Notification Manager & Controller
/// ════════════════════════════════════════════════════════════════

class NotificationManager : public std::enable_shared_from_this<NotificationManager> {
private:
    std::vector<NotificationItem> items_;
    std::vector<NotificationItem> active_push_toasts_;
    std::vector<std::function<void()>> listeners_;
    bool is_center_open_ = false;

public:
    void addListener(std::function<void()> l) {
        listeners_.push_back(std::move(l));
    }

    void notifyListeners() {
        for (auto& l : listeners_) {
            if (l) l();
        }
    }

    void post(NotificationItem item, bool show_push_toast = true) {
        items_.insert(items_.begin(), item);
        if (show_push_toast) {
            active_push_toasts_.insert(active_push_toasts_.begin(), item);
            if (active_push_toasts_.size() > 4) {
                active_push_toasts_.pop_back(); // Max 4 concurrent floating push toasts
            }
        }
        notifyListeners();
    }

    void dismissPushToast(const std::string& id) {
        auto it = std::remove_if(active_push_toasts_.begin(), active_push_toasts_.end(),
                                [&](const NotificationItem& n) { return n.id == id; });
        if (it != active_push_toasts_.end()) {
            active_push_toasts_.erase(it, active_push_toasts_.end());
            notifyListeners();
        }
    }

    void markAsRead(const std::string& id) {
        for (auto& item : items_) {
            if (item.id == id) {
                item.is_read = true;
                break;
            }
        }
        notifyListeners();
    }

    void markAllAsRead() {
        for (auto& item : items_) {
            item.is_read = true;
        }
        notifyListeners();
    }

    void remove(const std::string& id) {
        dismissPushToast(id);
        auto it = std::remove_if(items_.begin(), items_.end(),
                                [&](const NotificationItem& n) { return n.id == id; });
        if (it != items_.end()) {
            items_.erase(it, items_.end());
            notifyListeners();
        }
    }

    void clearAll() {
        items_.clear();
        active_push_toasts_.clear();
        notifyListeners();
    }

    [[nodiscard]] int getUnreadCount() const {
        int count = 0;
        for (const auto& item : items_) {
            if (!item.is_read) count++;
        }
        return count;
    }

    [[nodiscard]] const std::vector<NotificationItem>& getAllItems() const { return items_; }
    [[nodiscard]] const std::vector<NotificationItem>& getActivePushToasts() const { return active_push_toasts_; }

    void toggleCenter() {
        is_center_open_ = !is_center_open_;
        notifyListeners();
    }

    void openCenter() {
        if (!is_center_open_) {
            is_center_open_ = true;
            notifyListeners();
        }
    }

    void closeCenter() {
        if (is_center_open_) {
            is_center_open_ = false;
            notifyListeners();
        }
    }

    [[nodiscard]] bool isCenterOpen() const { return is_center_open_; }
};

/// ════════════════════════════════════════════════════════════════
/// Notification Bell Widget Implementation
/// ════════════════════════════════════════════════════════════════

class NotificationBellWidget : public StatelessWidget {
public:
    std::shared_ptr<NotificationManager> manager;
    std::string bell_icon = "🔔";

    explicit NotificationBellWidget(std::shared_ptr<NotificationManager> mgr, std::string icon = "🔔")
        : manager(std::move(mgr)), bell_icon(std::move(icon)) {}
    NotificationBellWidget(Key k, std::shared_ptr<NotificationManager> mgr, std::string icon = "🔔")
        : StatelessWidget(std::move(k)), manager(std::move(mgr)), bell_icon(std::move(icon)) {}

    WidgetPtr build(BuildContext&) override;
    [[nodiscard]] std::string_view typeName() const override { return "NotificationBell"; }
};

/// ════════════════════════════════════════════════════════════════
/// Notification Overlay Widget Implementation
/// ════════════════════════════════════════════════════════════════

class NotificationOverlayWidget : public StatefulWidget {
public:
    WidgetPtr body;                              ///< Main page body to wrap
    std::shared_ptr<NotificationManager> manager;

    NotificationOverlayWidget(WidgetPtr body_, std::shared_ptr<NotificationManager> mgr)
        : body(std::move(body_)), manager(std::move(mgr)) {}
    NotificationOverlayWidget(Key k, WidgetPtr body_, std::shared_ptr<NotificationManager> mgr)
        : StatefulWidget(std::move(k)), body(std::move(body_)), manager(std::move(mgr)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "NotificationOverlay"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Structs (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct NotificationBell {
    Key key = Key::none();
    std::shared_ptr<NotificationManager> manager = nullptr;
    std::string bell_icon = "🔔";

    operator WidgetPtr() const {
        return std::make_shared<NotificationBellWidget>(key, manager, bell_icon);
    }
};

struct NotificationOverlay {
    Key key = Key::none();
    WidgetPtr body = nullptr;
    std::shared_ptr<NotificationManager> manager = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<NotificationOverlayWidget>(key, body, manager);
    }
};

} // namespace enki
