#pragma once
/// @file selectable_text.hpp
/// @brief SelectableText Declarative Wrapper for ENKI Framework.
/// Enables mouse drag selection, word/all selection, and clipboard copy.
///
/// Built cleanly on top of the unified Text engine (RenderParagraph).
/// 100% C++20 Declarative Syntax.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/text.hpp"

namespace enki {

/// @brief Declarative convenience proxy for creating a selectable Text widget.
struct SelectableText {
    std::string                         text = "";
    std::optional<TextStyle>            style = std::nullopt;
    std::optional<Color>                color = std::nullopt;
    std::optional<float>                font_size = std::nullopt;
    std::optional<FontWeight>           font_weight = std::nullopt;
    std::optional<FontStyle>            font_style = std::nullopt;
    std::optional<std::string>          font_family = std::nullopt;
    Color                               selection_color = 0x6038BDF8;
    std::optional<TextAlign>            text_align = std::nullopt;
    std::optional<TextDirection>        text_direction = std::nullopt;
    std::optional<TextOverflow>         overflow = std::nullopt;
    std::optional<size_t>               max_lines = std::nullopt;
    std::optional<bool>                 soft_wrap = std::nullopt;
    std::function<void(TextSelection)>  on_selection_changed = nullptr;
    Key                                 key = Key::none();

    operator WidgetPtr() const {
        TextProps p;
        p.text = text;
        p.style = style;
        p.color = color;
        p.font_size = font_size;
        p.font_weight = font_weight;
        p.font_style = font_style;
        p.font_family = font_family;
        p.text_align = text_align;
        p.text_direction = text_direction;
        p.overflow = overflow;
        p.max_lines = max_lines;
        p.soft_wrap = soft_wrap;
        p.selectable = true;
        p.selection_color = selection_color;
        p.on_selection_changed = on_selection_changed;
        p.key = key;
        return enki::text(std::move(p));
    }
};

/// @brief Factory function to create a selectable text widget.
inline std::shared_ptr<Text> selectableText(std::string text) {
    TextProps p;
    p.text = std::move(text);
    p.selectable = true;
    return enki::text(std::move(p));
}

/// @brief Factory function to create a selectable text widget with custom style.
inline std::shared_ptr<Text> selectableText(std::string text, TextStyle style) {
    TextProps p;
    p.text = std::move(text);
    p.style = std::move(style);
    p.selectable = true;
    return enki::text(std::move(p));
}

/// @brief Factory function to create a selectable text widget with full props.
inline std::shared_ptr<Text> selectableText(TextProps props) {
    props.selectable = true;
    return enki::text(std::move(props));
}

} // namespace enki
