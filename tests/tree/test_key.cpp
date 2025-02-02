#include "enki/tree/key.hpp"
#include <cassert>
#include <string>

using namespace enki;

void test_keys() {
    auto k1 = Key::value(42);
    auto k2 = Key::value(42);
    auto k3 = Key::string("test");
    auto k4 = Key::string("test");
    auto k5 = Key::unique();
    auto k6 = Key::unique();

    assert(k1 == k2);
    assert(k3 == k4);
    assert(k5 != k6);
    assert(k1 != k3);
    assert(Key::none() == Key::none());
}

int main() {
    test_keys();
    return 0;
}
