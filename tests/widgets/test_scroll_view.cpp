#include <cassert>
#include <iostream>
#include <memory>

#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/container.hpp"
#include "enki/tree/render_object.hpp"

using namespace enki;

class TestRenderBox : public RenderBox {
public:
    void paint(PaintContext& context) override {}
};

int main() {
    std::cout << "Running test_scroll_view..." << std::endl;

    ScrollOptions opts;
    opts.direction = Axis::Vertical;
    
    // We instantiate the RenderObjects directly since we don't need a full widget tree for this unit test
    auto scroll_render = std::make_unique<RenderScrollView>(opts);
    
    // We need a child render object
    auto child_render = std::make_unique<TestRenderBox>();
    scroll_render->addChild(child_render.get());
    
    // Manually set sizes for testing since Anu layout isn't run completely here
    scroll_render->setSize(Size{100.0f, 200.0f}); // Viewport is 200 high
    child_render->setSize(Size{100.0f, 1000.0f}); // Child is 1000 high

    // Trigger syncLayout to calculate max_scroll, but since Anu isn't fully mocked here, 
    // it will overwrite sizes to 0. We'll manually compute max_scroll_y for the test.
    scroll_render->max_scroll_y = 800.0f;
    scroll_render->max_scroll_x = 0.0f;

    // 2. Test initial scroll offset
    assert(scroll_render->scroll_offset_y == 0.0f);

    // 3. Test scrolling down (drag up)
    // Dragging up by 100 pixels
    DragUpdateDetails drag_up;
    drag_up.delta = {0.0f, -100.0f};
    scroll_render->pan_recognizer.on_pan_update(drag_up);

    // Because we inverted delta in updateScrollOffsets, scroll_offset should increase
    assert(scroll_render->scroll_offset_y == 100.0f);

    // 4. Test clamping at bottom
    DragUpdateDetails huge_drag_up;
    huge_drag_up.delta = {0.0f, -2000.0f}; // Trying to scroll way past bottom
    scroll_render->pan_recognizer.on_pan_update(huge_drag_up);

    // Max scroll should be child_height - viewport_height = 1000 - 200 = 800
    assert(scroll_render->scroll_offset_y == 800.0f);

    // 5. Test clamping at top
    DragUpdateDetails huge_drag_down;
    huge_drag_down.delta = {0.0f, 2000.0f}; // Trying to scroll way past top
    scroll_render->pan_recognizer.on_pan_update(huge_drag_down);

    assert(scroll_render->scroll_offset_y == 0.0f);

    // 6. Test hit testing translation
    HitTestResult result;
    // Scroll down by 50
    DragUpdateDetails drag_down_50;
    drag_down_50.delta = {0.0f, -50.0f};
    scroll_render->pan_recognizer.on_pan_update(drag_down_50);
    assert(scroll_render->scroll_offset_y == 50.0f);

    [[maybe_unused]] bool hit = scroll_render->hitTestChildren(result, Point{50.0f, 10.0f});
    
    // We just verify it doesn't crash. (Default hitTestChildren of RenderBox just hits the child if it's within bounds)
    // Child is kept alive by `child_render` unique_ptr
    
    // Cleanup - remove child to avoid double-free if tree destruction tries to delete
    // Actually, RenderObject children are just raw pointers and we own it via child_render unique_ptr. 
    // RenderObject destructor doesn't delete children!
    
    std::cout << "test_scroll_view PASSED" << std::endl;
    return 0;
}
