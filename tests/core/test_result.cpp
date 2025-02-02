/// @file test_result.cpp
/// @brief Unit tests for Result<T> type.

#include "enki/core/result.hpp"
#include <cassert>
#include <cstdio>
#include <string>

using namespace enki;

void test_result_ok() {
    auto r = Result<int>::ok(42);
    assert(r.isOk());
    assert(!r.isErr());
    assert(r.value() == 42);
    assert(static_cast<bool>(r) == true);
    printf("  [PASS] Result::ok\n");
}

void test_result_err() {
    auto r = Result<int>::err(ErrorCode::NotFound, "item not found");
    assert(!r.isOk());
    assert(r.isErr());
    assert(r.error().code == ErrorCode::NotFound);
    assert(r.error().message == "item not found");
    assert(static_cast<bool>(r) == false);
    printf("  [PASS] Result::err\n");
}

void test_result_value_or() {
    auto ok = Result<int>::ok(42);
    assert(ok.valueOr(0) == 42);

    auto err = Result<int>::err(ErrorCode::NotFound);
    assert(err.valueOr(0) == 0);

    printf("  [PASS] Result::valueOr\n");
}

void test_result_map() {
    auto r = Result<int>::ok(21);
    auto doubled = r.map([](int v) { return v * 2; });
    assert(doubled.isOk());
    assert(doubled.value() == 42);

    auto err = Result<int>::err(ErrorCode::NotFound);
    auto mapped = err.map([](int v) { return v * 2; });
    assert(mapped.isErr());
    assert(mapped.error().code == ErrorCode::NotFound);

    printf("  [PASS] Result::map\n");
}

void test_result_and_then() {
    auto r = Result<int>::ok(21);
    auto chained = r.andThen([](int v) -> Result<std::string> {
        return Result<std::string>::ok(std::to_string(v * 2));
    });
    assert(chained.isOk());
    assert(chained.value() == "42");

    auto err = Result<int>::err(ErrorCode::InvalidArgument, "bad");
    auto chained2 = err.andThen([](int v) -> Result<std::string> {
        return Result<std::string>::ok("should not reach");
    });
    assert(chained2.isErr());
    assert(chained2.error().code == ErrorCode::InvalidArgument);

    printf("  [PASS] Result::andThen\n");
}

void test_result_void() {
    auto ok = Result<void>::ok();
    assert(ok.isOk());
    assert(!ok.isErr());

    auto err = Result<void>::err(ErrorCode::PlatformError, "crash");
    assert(err.isErr());
    assert(err.error().code == ErrorCode::PlatformError);

    printf("  [PASS] Result<void>\n");
}

void test_error_code_string() {
    Error e(ErrorCode::OutOfMemory, "oops");
    assert(e.codeString() == "OutOfMemory");
    assert(!e.isNone());

    Error none;
    assert(none.isNone());

    printf("  [PASS] Error::codeString\n");
}

int main() {
    printf("=== Result Tests ===\n");

    test_result_ok();
    test_result_err();
    test_result_value_or();
    test_result_map();
    test_result_and_then();
    test_result_void();
    test_error_code_string();

    printf("=== All Result tests passed! ===\n");
    return 0;
}
