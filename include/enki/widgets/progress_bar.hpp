#pragma once
/// @file progress_bar.hpp
/// @brief Advanced ProgressBar widget with Determinate/Indeterminate modes, Gradients, and SkSL Shader Injection.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/widgets/flexbox.hpp"
#include <string>
#include <vector>
#include <memory>

namespace enki {

class ProgressBarWidget : public StatefulWidget {
public:
    float value = 0.0f; // 0.0f to 1.0f
    
    float height = 8.0f;
    float border_radius = 4.0f;
    
    Color background_color = 0xFF1E293B; // Dark slate
    Color progress_color   = 0xFF3B82F6; // Blue 500
    std::vector<Color> gradient_colors = {}; // Optional multi-stop gradient
    
    Color glow_color = 0x00000000;
    float glow_blur  = 0.0f;
    
    bool indeterminate = false;
    std::string custom_shader = ""; // SkSL Runtime Shader
    
    bool show_label = false;
    std::string label_format = "{percent}%";
    
    float min_width = 100.0f;

    explicit ProgressBarWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ProgressBar"; }
};

struct ProgressBar {
    float value = 0.0f; // 0.0f to 1.0f
    
    float height = 8.0f;
    float border_radius = 4.0f;
    
    Color background_color = 0xFF1E293B; // Dark slate
    Color progress_color   = 0xFF3B82F6; // Blue 500
    std::vector<Color> gradient_colors = {}; // Optional multi-stop gradient
    
    Color glow_color = 0x00000000;
    float glow_blur  = 0.0f;
    
    bool indeterminate = false;
    std::string custom_shader = ""; // SkSL Runtime Shader
    
    bool show_label = false;
    std::string label_format = "{percent}%";
    
    float min_width = 100.0f;
    
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<ProgressBarWidget>(key);
        w->value = value;
        w->height = height;
        w->border_radius = border_radius;
        w->background_color = background_color;
        w->progress_color = progress_color;
        w->gradient_colors = gradient_colors;
        w->glow_color = glow_color;
        w->glow_blur = glow_blur;
        w->indeterminate = indeterminate;
        w->custom_shader = custom_shader;
        w->show_label = show_label;
        w->label_format = label_format;
        w->min_width = min_width;
        return w;
    }
};

} // namespace enki
