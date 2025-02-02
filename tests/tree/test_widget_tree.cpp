#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include <cassert>

using namespace enki;

class TestWidget : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return nullptr;
    }
    std::string_view typeName() const override { return "TestWidget"; }
};

void test_widget_creation() {
    auto w = std::make_shared<TestWidget>();
    auto e = w->createElement();
    assert(e != nullptr);
}

int main() {
    test_widget_creation();
    return 0;
}
