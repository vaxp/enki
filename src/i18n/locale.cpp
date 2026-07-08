/// @file locale.cpp
/// @brief Native OS Locale querying and BCP-47 / POSIX parsing.

#include "enki/i18n/locale.hpp"
#include <cstdlib>
#include <cctype>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace enki {

Locale Locale::fromTag(std::string_view tag) {
    if (tag.empty()) {
        return Locale("en", "US");
    }

    // Strip trailing codeset like ".UTF-8" or "@euro"
    auto dot_pos = tag.find('.');
    if (dot_pos != std::string_view::npos) {
        tag = tag.substr(0, dot_pos);
    }
    auto at_pos = tag.find('@');
    if (at_pos != std::string_view::npos) {
        tag = tag.substr(0, at_pos);
    }

    // Split language and region (by '-' or '_')
    auto sep = tag.find_first_of("-_");
    if (sep == std::string_view::npos) {
        std::string lang(tag);
        for (char& c : lang) c = static_cast<char>(std::tolower(c));
        return Locale(lang);
    }

    std::string lang(tag.substr(0, sep));
    std::string country(tag.substr(sep + 1));

    for (char& c : lang) c = static_cast<char>(std::tolower(c));
    for (char& c : country) c = static_cast<char>(std::toupper(c));

    return Locale(lang, country);
}

Locale Locale::system() {
#if defined(_WIN32)
    WCHAR buffer[LOCALE_NAME_MAX_LENGTH] = {0};
    if (GetUserDefaultLocaleName(buffer, LOCALE_NAME_MAX_LENGTH) > 0) {
        char utf8[128] = {0};
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, utf8, sizeof(utf8), nullptr, nullptr);
        return Locale::fromTag(utf8);
    }
#else
    const char* env = std::getenv("LC_ALL");
    if (!env || !*env) env = std::getenv("LC_MESSAGES");
    if (!env || !*env) env = std::getenv("LANG");
    if (env && *env) {
        return Locale::fromTag(env);
    }
#endif
    return Locale("en", "US");
}

} // namespace enki
