#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/tag_input.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class TagInputDemoState : public State {
    std::vector<std::string> tags_ = {"VAXP-OS", "ZeroCopy", "Skia", "C++20", "ENKI"};

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("TagInput Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Dynamic Tokenized Chip Management (Type and press Enter) (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        std::string summary = "Active Tags (" + std::to_string(tags_.size()) + "): ";
        for (size_t i = 0; i < tags_.size(); ++i) {
            if (i > 0) summary += ", ";
            summary += tags_[i];
        }

        auto status = text(summary, {
            .color = 0xFF38BDF8,
            .font_size = 13.5f,
            .font_weight = FontWeight::Medium
        });

        auto ti = tagInput({
            .tags = tags_,
            .placeholder = "Type tag name and hit Enter...",
            .on_tags_changed = [this](const std::vector<std::string>& t) {
                tags_ = t;
                setState([]{});
            },
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {title, subtitle, ti, status}
        });

        return container(ContainerProps{
            .color = 0xFF0B1320,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(40.0f),
            .child = main_col
        });
    }
};

class TagInputDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "TagInputDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<TagInputDemoState>(); }
};

int main() {
    std::cout << "=== ENKI TagInput Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — TagInput Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<TagInputDemoApp>(), config);
}
