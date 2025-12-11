#pragma once
/// @file tag_input.hpp
/// @brief TagInput widget for ENKI Framework.
/// Interactive multi-tag chip input with auto-wrap, inline text entry, and removable chips.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace enki {

struct TagInputProps {
    std::vector<std::string>                    tags;
    std::string                                 placeholder = "Add tag and press Enter...";
    int                                         max_tags = 20;
    bool                                        allow_duplicates = false;

    Color                                       chip_background = 0x2600E5FF;
    Color                                       chip_border_color = 0x6600E5FF;
    Color                                       chip_text_color = 0xFF38BDF8;
    Color                                       delete_icon_color = 0xFF94A3B8;
    Color                                       container_background = 0x59000000;
    Color                                       border_color = 0x3300E5FF;
    float                                       border_radius = 10.0f;
    float                                       min_height = 42.0f;

    std::function<void(const std::vector<std::string>&)> on_tags_changed;

    operator WidgetPtr() const;
};

class TagInputWidget : public StatefulWidget {
public:
    TagInputProps props;

    explicit TagInputWidget(TagInputProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "TagInput"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr tagInput(TagInputProps props) {
    return std::make_shared<TagInputWidget>(std::move(props));
}

} // namespace enki
