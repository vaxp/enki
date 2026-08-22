#pragma once
/// @file radio.hpp
/// @brief Radio widget for single-choice selection.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

/// @brief A material-style radio button for single selection from a group.
///
/// Uses `int` for the value and group_value types for simplicity and performance.
class RadioWidget : public StatefulWidget {
public:
    int value;
    int group_value;
    std::function<void(int)> on_changed;
    
    float size = 20.0f;                             
    float inner_size = 10.0f;                       
    float border_width = 2.0f;                      
    
    Color active_color = 0xFF2563EB;                
    Color inactive_color = 0xFF64748B;              
    Color hover_color = 0xFF3B82F6;                 
    Color bg_color = 0x00000000;                    
    
    bool disabled = false;                          
    Color disabled_color = 0xFF94A3B8;              

    RadioWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Radio"; }
};

struct Radio {
    int value = 0;
    int group_value = 0;
    std::function<void(int)> on_changed = nullptr;
    
    float size = 20.0f;                             
    float inner_size = 10.0f;                       
    float border_width = 2.0f;                      
    
    Color active_color = 0xFF2563EB;                
    Color inactive_color = 0xFF64748B;              
    Color hover_color = 0xFF3B82F6;                 
    Color bg_color = 0x00000000;                    
    
    bool disabled = false;                          
    Color disabled_color = 0xFF94A3B8;              
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<RadioWidget>(key);
        w->value = value;
        w->group_value = group_value;
        w->on_changed = on_changed;
        w->size = size;
        w->inner_size = inner_size;
        w->border_width = border_width;
        w->active_color = active_color;
        w->inactive_color = inactive_color;
        w->hover_color = hover_color;
        w->bg_color = bg_color;
        w->disabled = disabled || on_changed == nullptr;
        w->disabled_color = disabled_color;
        return w;
    }
};

} // namespace enki
