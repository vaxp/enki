#pragma once
/// @file breadcrumb.hpp
/// @brief Breadcrumb — hierarchical navigation path indicator.
///
/// Features:
///   - Horizontal sequence of clickable labels with separators.
///   - Last item is active (non-clickable, distinct color).
///   - Custom separator string.
///   - Hover feedback on clickable items.
///   - Fluent builder API.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// BreadcrumbItem
// ════════════════════════════════════════════════════════════════

struct BreadcrumbItem {
    std::string          label;
    std::function<void()> on_tap;  ///< nullptr = non-clickable (last item)
};

// ════════════════════════════════════════════════════════════════
// BreadcrumbOptions
// ════════════════════════════════════════════════════════════════

struct BreadcrumbProps {
    Key key = Key::none();
    std::vector<BreadcrumbItem> items;
    Color active_color      = 0xFFF1F5F9;   ///< Last (current) item color
    Color inactive_color    = 0xFF64748B;   ///< Ancestor items color
    Color hover_color       = 0xFF818CF8;   ///< Hover color on clickable items
    Color separator_color   = 0xFF475569;

    std::string separator   = "/";
    float font_size         = 13.0f;
    float separator_font_size = 12.0f;
    float item_spacing      = 8.0f;
    float separator_spacing = 8.0f;
    bool  bold_active       = true;

};

// ════════════════════════════════════════════════════════════════
// Breadcrumb Widget
// ════════════════════════════════════════════════════════════════

/// @brief Builds a horizontal row of labeled path items.
class Breadcrumb : public StatelessWidget {
public:
    std::vector<BreadcrumbItem> items;
    BreadcrumbProps            options;

    Breadcrumb() = default;
    explicit Breadcrumb(std::vector<BreadcrumbItem> items, BreadcrumbProps opt = {})
        : items(std::move(items)), options(std::move(opt)) {}
    
    Breadcrumb(Key k, BreadcrumbProps opt) : StatelessWidget(std::move(k)), items(std::move(opt.items)), options(std::move(opt)) {}

    // Fluent API
    Breadcrumb& activeColor(Color c)     { options.active_color = c;   return *this; }
    Breadcrumb& inactiveColor(Color c)   { options.inactive_color = c; return *this; }
    Breadcrumb& separator(std::string s) { options.separator = std::move(s); return *this; }
    Breadcrumb& fontSize(float f)        { options.font_size = f;      return *this; }

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Breadcrumb"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Breadcrumb> breadcrumb(
        std::vector<BreadcrumbItem> items,
        BreadcrumbProps options = {}) {
    return std::make_shared<Breadcrumb>(std::move(items), std::move(options));
}

inline std::shared_ptr<Breadcrumb> breadcrumb(BreadcrumbProps props) {
    return std::make_shared<Breadcrumb>(std::move(props.key), std::move(props));
}

} // namespace enki
