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

/// Options for configuring a ProgressBar
struct ProgressBarProps {
    Key key = Key::none();
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
};

/// A linear progress indicator widget
class ProgressBar : public StatefulWidget {
public:
    float value; // 0.0f to 1.0f
    ProgressBarProps options;

    explicit ProgressBar(float value = 0.0f, ProgressBarProps options = ProgressBarProps())
        : value(value), options(std::move(options)) {}

    explicit ProgressBar(Key key, float value, ProgressBarProps options)
        : StatefulWidget(std::move(key)), value(value), options(std::move(options)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ProgressBar"; }
};

/// Helper function to construct a ProgressBar widget
inline std::shared_ptr<ProgressBar> progressBar(float value = 0.0f, ProgressBarProps options = ProgressBarProps()) {
    return std::make_shared<ProgressBar>(value, std::move(options));
}

inline std::shared_ptr<ProgressBar> progressBar(ProgressBarProps props) {
    return std::make_shared<ProgressBar>(std::move(props.key), props.value, std::move(props));
}

} // namespace enki
