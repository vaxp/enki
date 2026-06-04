#pragma once
/// @file terminal.hpp
/// @brief Modern Terminal styling, ANSI Truecolor, glyphs, and structured outputs.

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace enki::cli {

enum class Color {
    Default,
    Bold,
    Dim,
    Cyan,
    BrightCyan,
    Green,
    BrightGreen,
    Yellow,
    BrightYellow,
    Red,
    BrightRed,
    Magenta,
    BrightMagenta,
    Gray,
    White
};

class Terminal {
public:
    static void init() {
#if defined(_WIN32)
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
    }

    static std::string colorCode(Color c) {
        switch (c) {
            case Color::Bold:          return "\033[1m";
            case Color::Dim:           return "\033[2m";
            case Color::Cyan:          return "\033[36m";
            case Color::BrightCyan:    return "\033[96m";
            case Color::Green:         return "\033[32m";
            case Color::BrightGreen:   return "\033[92m";
            case Color::Yellow:        return "\033[33m";
            case Color::BrightYellow:  return "\033[93m";
            case Color::Red:           return "\033[31m";
            case Color::BrightRed:     return "\033[91m";
            case Color::Magenta:       return "\033[35m";
            case Color::BrightMagenta: return "\033[95m";
            case Color::Gray:          return "\033[90m";
            case Color::White:         return "\033[97m";
            default:                   return "\033[0m";
        }
    }

    static std::string resetCode() {
        return "\033[0m";
    }

    static std::string style(const std::string& text, Color c, bool bold = false) {
        std::string res;
        if (bold) res += colorCode(Color::Bold);
        res += colorCode(c);
        res += text;
        res += resetCode();
        return res;
    }

    static void banner() {
        std::cout << style("  ███████╗███╗   ██╗██╗  ██╗██╗\n", Color::BrightCyan, true);
        std::cout << style("  ██╔════╝████╗  ██║██║ ██╔╝██║\n", Color::BrightCyan, true);
        std::cout << style("  █████╗  ██╔██╗ ██║█████╔╝ ██║\n", Color::Cyan, true);
        std::cout << style("  ██╔══╝  ██║╚██╗██║██╔═██╗ ██║\n", Color::BrightCyan, true);
        std::cout << style("  ███████╗██║ ╚████║██║  ██╗██║\n", Color::BrightCyan, true);
        std::cout << style("  ╚══════╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝\n", Color::Cyan, true);
        std::cout << style("  High-Performance Cross-Platform App Engine CLI\n\n", Color::Gray);
    }

    static void header(const std::string& title) {
        std::cout << style("==> ", Color::BrightCyan, true)
                  << style(title, Color::White, true) << "\n";
    }

    static void ok(const std::string& label, const std::string& details = "") {
        std::cout << "  " << style("[✓] ", Color::BrightGreen, true)
                  << std::left << std::setw(24) << label;
        if (!details.empty()) {
            std::cout << " " << style(details, Color::Gray);
        }
        std::cout << "\n";
    }

    static void warn(const std::string& label, const std::string& details = "") {
        std::cout << "  " << style("[!] ", Color::BrightYellow, true)
                  << std::left << std::setw(24) << label;
        if (!details.empty()) {
            std::cout << " " << style(details, Color::Yellow);
        }
        std::cout << "\n";
    }

    static void fail(const std::string& label, const std::string& details = "") {
        std::cout << "  " << style("[✗] ", Color::BrightRed, true)
                  << std::left << std::setw(24) << label;
        if (!details.empty()) {
            std::cout << " " << style(details, Color::Red);
        }
        std::cout << "\n";
    }

    static void step(int cur, int total, const std::string& msg) {
        std::string badge = "[" + std::to_string(cur) + "/" + std::to_string(total) + "] ";
        std::cout << style(badge, Color::BrightCyan, true)
                  << style(msg, Color::White) << "...\n";
    }

    static void info(const std::string& msg) {
        std::cout << style("• ", Color::BrightCyan) << msg << "\n";
    }
};

} // namespace enki::cli
