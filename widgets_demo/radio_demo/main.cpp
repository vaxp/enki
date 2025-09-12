#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/radio.hpp"
#include "enki/state/state.hpp"

#include <iostream>

using namespace enki;

class RadioDemoWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "RadioDemoWidget"; }
    std::unique_ptr<State> createState() override;
};

class RadioDemoState : public State {
    int group_value_ = 1;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("Radio Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f });
        
        auto create_radio_row = [this](int value, const std::string& label) {
            return row({
                .align_items = Align::Center,
                .gap = StyleValue::point(16.0f),
                .children = {
                    Radio {
                        .value = value,
                        .group_value = group_value_,
                        .on_changed = [this](int val){
                            setState([this, val]{ group_value_ = val; });
                        }
                    },
                    text(label, { .color = 0xFFCCCCCC })
                }
            });
        };

        auto r1 = create_radio_row(1, "Option 1 (Selected by default)");
        auto r2 = create_radio_row(2, "Option 2");
        auto r3 = create_radio_row(3, "Option 3");
        
        auto row_disabled = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {
                Radio {
                    .value = 4,
                    .group_value = group_value_,
                    .on_changed = nullptr,
                    .disabled = true
                },
                text("Disabled Option", { .color = 0xFF888888 })
            }
        });

        return container({
            .color = 0xFF1E1E1E,
            .child = column({
                .gap = StyleValue::point(20.0f),
                .padding = StyleInsets::all(32.0f),
                .children = {title, r1, r2, r3, row_disabled}
            })
        });
    }
};

std::unique_ptr<State> RadioDemoWidget::createState() {
    return std::make_unique<RadioDemoState>();
}

int main() {
    std::cout << "Starting Radio Demo...\n";
    AppConfig config;
    config.title = "ENKI Radio Demo";
    config.width = 600;
    config.height = 400;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    
    return runApp(std::make_shared<RadioDemoWidget>(), config);
}
