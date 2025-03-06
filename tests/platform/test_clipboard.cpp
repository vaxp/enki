/// @file test_clipboard.cpp
/// @brief Unit tests for ClipboardData, MIME type handling, and URI list parsing.

#include "enki/platform/clipboard.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

void test_clipboard_data_text() {
    using namespace enki;

    ClipboardData data;
    assert(data.empty());
    assert(!data.hasText());

    data.setText("Hello, Enki Shell!");
    assert(!data.empty());
    assert(data.hasText());
    assert(data.getText() == "Hello, Enki Shell!");
    assert(data.hasFormat(mime::TextPlainUtf8));
    assert(data.hasFormat(mime::TextPlain));

    std::cout << "[PASS] test_clipboard_data_text\n";
}

void test_clipboard_data_uris() {
    using namespace enki;

    ClipboardData data;
    std::vector<std::string> sample_uris = {
        "file:///home/user/document.pdf",
        "file:///home/user/image.png",
        "https://enki-framework.org"
    };

    data.setUris(sample_uris);
    assert(!data.empty());
    assert(data.hasUris());
    assert(data.hasFormat(mime::TextUriList));

    auto retrieved = data.getUris();
    assert(retrieved.size() == 3);
    assert(retrieved[0] == "file:///home/user/document.pdf");
    assert(retrieved[1] == "file:///home/user/image.png");
    assert(retrieved[2] == "https://enki-framework.org");

    std::cout << "[PASS] test_clipboard_data_uris\n";
}

void test_clipboard_raw_mime() {
    using namespace enki;

    ClipboardData data;
    std::vector<uint8_t> png_magic = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    data.setRaw(mime::ImagePng, png_magic);

    assert(data.hasFormat(mime::ImagePng));
    assert(data.getRaw(mime::ImagePng) == png_magic);

    auto formats = data.formats();
    assert(formats.size() == 1);
    assert(formats[0] == mime::ImagePng);

    data.clear();
    assert(data.empty());
    assert(!data.hasFormat(mime::ImagePng));

    std::cout << "[PASS] test_clipboard_raw_mime\n";
}

int main() {
    test_clipboard_data_text();
    test_clipboard_data_uris();
    test_clipboard_raw_mime();
    std::cout << "All Clipboard unit tests passed successfully!\n";
    return 0;
}
