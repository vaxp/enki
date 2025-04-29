#include "enki/app/app.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/rendering/font_manager.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class IconDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "IconDemoApp"; }
};

class IconDemoState : public State {
public:
    void initState() override {
        State::initState(); // Required for lifecycle
        
        // Load custom icon font
        bool loaded = FontManager::loadFont("assets/fonts/MaterialIcons-Regular.ttf", "Material Icons");
        if (!loaded) {
            std::cerr << "Warning: Could not load Material Icons font." << std::endl;
        }
    }

    WidgetPtr build(BuildContext& context) override {
        // SVG Icons Row
        auto svg_label_text = text("SVG Icons");
        svg_label_text->fontSize(18.0f).color(0xFF94A3B8);
        auto svg_label = container(std::static_pointer_cast<Widget>(svg_label_text));
        svg_label->margin(StyleInsets::symmetric(0.0f, 10.0f));
        
        std::vector<WidgetPtr> svg_icons;
        svg_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::SVG::play())->size(32.0f)->color(0xFF10B981)));
        svg_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::SVG::check())->size(48.0f)->color(0xFF3B82F6)));
        svg_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::SVG::play())->size(64.0f)->color(0xFFEF4444)));
        auto svg_row = row(svg_icons);
        svg_row->gap(20.0f).alignItems(Align::Center);
        
        std::vector<WidgetPtr> svg_section_children;
        svg_section_children.push_back(std::static_pointer_cast<Widget>(svg_label));
        svg_section_children.push_back(std::static_pointer_cast<Widget>(svg_row));
        auto svg_section = column(svg_section_children);

        // Material Icons Row
        auto font_label_text = text("Material Icons (Font)");
        font_label_text->fontSize(18.0f).color(0xFF94A3B8);
        auto font_label = container(std::static_pointer_cast<Widget>(font_label_text));
        font_label->margin(StyleInsets::symmetric(0.0f, 20.0f));
        
        std::vector<WidgetPtr> font_icons;
        font_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::Material::favorite())->size(32.0f)->color(0xFFEC4899)));
        font_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::Material::home())->size(48.0f)->color(0xFF8B5CF6)));
        font_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::Material::settings())->size(64.0f)->color(0xFF6366F1)));
        font_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::Material::search())->size(48.0f)->color(0xFFF59E0B)));
        font_icons.push_back(std::static_pointer_cast<Widget>(icon(Icons::Material::add())->size(32.0f)->color(0xFF10B981)));
        auto font_row = row(font_icons);
        font_row->gap(20.0f).alignItems(Align::Center);
        
        std::vector<WidgetPtr> font_section_children;
        font_section_children.push_back(std::static_pointer_cast<Widget>(font_label));
        font_section_children.push_back(std::static_pointer_cast<Widget>(font_row));
        auto font_section = column(font_section_children);

        auto title = text("ENKI Icon Widget");
        title->fontSize(28.0f).bold().color(0xFFFFFFFF);
        
        auto subtitle_text = text("Zero calculations. 100% Anu Layout. Native SVG & Fonts.");
        subtitle_text->fontSize(14.0f).color(0xFF94A3B8);
        
        auto subtitle = container(std::static_pointer_cast<Widget>(subtitle_text));
        subtitle->margin(StyleInsets::symmetric(0.0f, 20.0f));

        std::vector<WidgetPtr> main_children;
        main_children.push_back(std::static_pointer_cast<Widget>(title));
        main_children.push_back(std::static_pointer_cast<Widget>(subtitle));
        main_children.push_back(std::static_pointer_cast<Widget>(svg_section));
        main_children.push_back(std::static_pointer_cast<Widget>(font_section));

        // Main Layout
        auto main_col = column(main_children);
        
        main_col->alignItems(Align::Center).justifyContent(Justify::Center)
                .width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));

        auto bg = container(std::static_pointer_cast<Widget>(main_col));
        bg->color(0xFF0F172A) // Dark background
          .width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));

        return bg;
    }
};

std::unique_ptr<State> IconDemoApp::createState() {
    return std::make_unique<IconDemoState>();
}

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Icon Demo";
    config.width       = 800;
    config.height      = 600;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<IconDemoApp>(), config);
}
