#pragma once
/// @file checkbox.hpp
/// @brief Checkbox widget for boolean input.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

/// @brief A material-style checkbox for boolean selection.
class CheckboxWidget : public StatefulWidget {
public:
    bool value;
    std::function<void(bool)> on_changed;
    
    float size = 18.0f;                             
    float border_width = 2.0f;                      
    float border_radius = 4.0f;                     
    
    Color active_color = 0xFF2563EB;                
    Color check_color = 0xFFFFFFFF;                 
    
    Color border_color = 0xFF363B42;                
    Color hover_border_color = 0xFF58A6FF;          
    Color inactive_bg_color = 0x00000000;           
    
    bool disabled = false;                          
    Color disabled_color = 0xFF475569;              

    CheckboxWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Checkbox"; }
};

struct Checkbox {
    bool value = false;
    std::function<void(bool)> on_changed = nullptr;
    
    float size = 18.0f;                             
    float border_width = 2.0f;                      
    float border_radius = 4.0f;                     
    
    Color active_color = 0xFF2563EB;                
    Color check_color = 0xFFFFFFFF;                 
    
    Color border_color = 0xFF363B42;                
    Color hover_border_color = 0xFF58A6FF;          
    Color inactive_bg_color = 0x00000000;           
    
    bool disabled = false;                          
    Color disabled_color = 0xFF475569;              
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<CheckboxWidget>(key);
        w->value = value;
        w->on_changed = on_changed;
        w->size = size;
        w->border_width = border_width;
        w->border_radius = border_radius;
        w->active_color = active_color;
        w->check_color = check_color;
        w->border_color = border_color;
        w->hover_border_color = hover_border_color;
        w->inactive_bg_color = inactive_bg_color;
        w->disabled = disabled || on_changed == nullptr;
        w->disabled_color = disabled_color;
        return w;
    }
};

} // namespace enki
