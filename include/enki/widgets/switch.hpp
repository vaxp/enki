#pragma once
/// @file switch.hpp
/// @brief Switch widget for boolean toggles.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <functional>

namespace enki {

/// @brief A material/iOS-style switch for boolean selection.
class SwitchWidget : public StatefulWidget {
public:
    bool value;
    std::function<void(bool)> on_changed;
    
    float width = 44.0f;                            
    float height = 24.0f;                           
    float thumb_padding = 2.0f;                     
    
    Color active_color = 0xFF34C759;                
    Color active_thumb_color = 0xFFFFFFFF;          
    
    Color inactive_color = 0xFFE5E5EA;              
    Color inactive_thumb_color = 0xFFFFFFFF;        
    
    Color hover_color = 0xFF28A745;                 
    Color hover_inactive_color = 0xFFD1D1D6;        
    
    bool disabled = false;                          
    Color disabled_color = 0xFFF2F2F7;              
    Color disabled_thumb_color = 0xFFE5E5EA;        

    SwitchWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Switch"; }
};

struct Switch {
    bool value = false;
    std::function<void(bool)> on_changed = nullptr;
    
    float width = 44.0f;                            
    float height = 24.0f;                           
    float thumb_padding = 2.0f;                     
    
    Color active_color = 0xFF34C759;                
    Color active_thumb_color = 0xFFFFFFFF;          
    
    Color inactive_color = 0xFFE5E5EA;              
    Color inactive_thumb_color = 0xFFFFFFFF;        
    
    Color hover_color = 0xFF28A745;                 
    Color hover_inactive_color = 0xFFD1D1D6;        
    
    bool disabled = false;                          
    Color disabled_color = 0xFFF2F2F7;              
    Color disabled_thumb_color = 0xFFE5E5EA;        
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<SwitchWidget>(key);
        w->value = value;
        w->on_changed = on_changed;
        w->width = width;
        w->height = height;
        w->thumb_padding = thumb_padding;
        w->active_color = active_color;
        w->active_thumb_color = active_thumb_color;
        w->inactive_color = inactive_color;
        w->inactive_thumb_color = inactive_thumb_color;
        w->hover_color = hover_color;
        w->hover_inactive_color = hover_inactive_color;
        w->disabled = disabled || on_changed == nullptr;
        w->disabled_color = disabled_color;
        w->disabled_thumb_color = disabled_thumb_color;
        return w;
    }
};

} // namespace enki
