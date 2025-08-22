#pragma once
/// @file progress_ring.hpp
/// @brief Advanced ProgressRing widget with Determinate/Indeterminate modes, Gradients, and SkSL Shader Injection.
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

class ProgressRingWidget : public StatefulWidget {
public:
    float value = 0.0f; // 0.0f to 1.0f
    WidgetPtr child = nullptr;
    
    float size = 48.0f;
    float stroke_width = 6.0f;
    
    Color background_color = 0xFF1E293B; // Dark slate
    Color progress_color   = 0xFF3B82F6; // Blue 500
    std::vector<Color> gradient_colors = {}; // Optional sweep gradient
    
    Color glow_color = 0x00000000;
    float glow_blur  = 0.0f;
    
    bool round_cap   = true;
    float start_angle = -90.0f; // 12 o'clock
    
    bool indeterminate = false;
    std::string custom_shader = ""; // SkSL Runtime Shader

    explicit ProgressRingWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ProgressRing"; }
};

struct ProgressRing {
    float value = 0.0f; // 0.0f to 1.0f
    
    float size = 48.0f;
    float stroke_width = 6.0f;
    
    Color background_color = 0xFF1E293B; // Dark slate
    Color progress_color   = 0xFF3B82F6; // Blue 500
    std::vector<Color> gradient_colors = {}; // Optional sweep gradient
    
    Color glow_color = 0x00000000;
    float glow_blur  = 0.0f;
    
    bool round_cap   = true;
    float start_angle = -90.0f; // 12 o'clock
    
    bool indeterminate = false;
    std::string custom_shader = ""; // SkSL Runtime Shader
    
    WidgetPtr child = nullptr;
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<ProgressRingWidget>(key);
        w->value = value;
        w->child = child;
        w->size = size;
        w->stroke_width = stroke_width;
        w->background_color = background_color;
        w->progress_color = progress_color;
        w->gradient_colors = gradient_colors;
        w->glow_color = glow_color;
        w->glow_blur = glow_blur;
        w->round_cap = round_cap;
        w->start_angle = start_angle;
        w->indeterminate = indeterminate;
        w->custom_shader = custom_shader;
        return w;
    }
};

} // namespace enki
