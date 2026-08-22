#pragma once
/// @file breadcrumb.hpp
/// @brief Breadcrumb — hierarchical navigation path indicator.
///
/// Features:
///   - Horizontal sequence of clickable labels with separators.
///   - Last item is active (non-clickable, distinct color).
///   - Custom separator string.
///   - Hover feedback on clickable items.
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
    std::function<void()> on_tap = nullptr;  ///< nullptr = non-clickable (last item)
};

// ════════════════════════════════════════════════════════════════
// BreadcrumbProps
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
// Breadcrumb Implementation Widget
// ════════════════════════════════════════════════════════════════

/// @brief Builds a horizontal row of labeled path items.
class BreadcrumbWidget : public StatelessWidget {
public:
    BreadcrumbProps options;

    BreadcrumbWidget() = default;
    explicit BreadcrumbWidget(BreadcrumbProps opt)
        : StatelessWidget(opt.key), options(std::move(opt)) {}
    BreadcrumbWidget(Key k, BreadcrumbProps opt)
        : StatelessWidget(std::move(k)), options(std::move(opt)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Breadcrumb"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Breadcrumb Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Breadcrumb {
    Key key = Key::none();
    std::vector<BreadcrumbItem> items;
    Color active_color      = 0xFFF1F5F9;
    Color inactive_color    = 0xFF64748B;
    Color hover_color       = 0xFF818CF8;
    Color separator_color   = 0xFF475569;

    std::string separator   = "/";
    float font_size         = 13.0f;
    float separator_font_size = 12.0f;
    float item_spacing      = 8.0f;
    float separator_spacing = 8.0f;
    bool  bold_active       = true;

    operator WidgetPtr() const {
        BreadcrumbProps p;
        p.key = key;
        p.items = items;
        p.active_color = active_color;
        p.inactive_color = inactive_color;
        p.hover_color = hover_color;
        p.separator_color = separator_color;
        p.separator = separator;
        p.font_size = font_size;
        p.separator_font_size = separator_font_size;
        p.item_spacing = item_spacing;
        p.separator_spacing = separator_spacing;
        p.bold_active = bold_active;
        return std::make_shared<BreadcrumbWidget>(key, std::move(p));
    }
};

} // namespace enki
