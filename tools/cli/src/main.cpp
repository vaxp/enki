/// @file main.cpp
/// @brief ENKI Native CLI Command Dispatcher and Entry Point.

#include "core/terminal.hpp"
#include "core/env.hpp"
#include "commands/command.hpp"
#include "commands/doctor.hpp"
#include "commands/devices.hpp"
#include "commands/create.hpp"
#include "commands/run.hpp"
#include "commands/clean.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>

using namespace enki::cli;

static void showHelp(const std::vector<std::shared_ptr<Command>>& commands) {
    Terminal::banner();
    std::cout << Terminal::style("USAGE:\n", Color::Bold);
    std::cout << "  enki <COMMAND> [OPTIONS]\n\n";

    std::cout << Terminal::style("COMMANDS:\n", Color::Bold);
    for (const auto& cmd : commands) {
        std::string n = cmd->name();
        if (n.length() < 14) n.append(14 - n.length(), ' ');
        std::cout << "  " << Terminal::style(n, Color::BrightCyan, true)
                  << cmd->description() << "\n";
    }

    std::cout << "\n" << Terminal::style("OPTIONS:\n", Color::Bold);
    std::cout << "  -h, --help       Show help information\n";
    std::cout << "  -v, --version    Show ENKI CLI version\n\n";
    std::cout << Terminal::style("EXAMPLES:\n", Color::Bold);
    std::cout << "  enki doctor                   # Run diagnostics on compilers and SDKs\n";
    std::cout << "  enki devices                  # List available Desktop & Android devices\n";
    std::cout << "  enki create my_app            # Scaffold a new application\n";
    std::cout << "  enki run gallery android      # Build, package, sign, and run on Android\n";
    std::cout << "  enki run gallery desktop      # Build and run on Windows Desktop\n\n";
}

int main(int argc, char* argv[]) {
    Terminal::init();

    std::vector<std::shared_ptr<Command>> commands = {
        std::make_shared<DoctorCommand>(),
        std::make_shared<DevicesCommand>(),
        std::make_shared<CreateCommand>(),
        std::make_shared<RunCommand>(),
        std::make_shared<CleanCommand>()
    };

    if (argc < 2) {
        showHelp(commands);
        return 0;
    }

    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help" || arg1 == "help") {
        showHelp(commands);
        return 0;
    }

    if (arg1 == "-v" || arg1 == "--version" || arg1 == "version") {
        std::cout << "ENKI CLI version 0.2.0 (C++20 Native Toolchain)\n";
        return 0;
    }

    fs::path repo_root = Env::findEnkiRepoRoot();

    std::vector<std::string> cmd_args;
    for (int i = 2; i < argc; ++i) {
        cmd_args.push_back(argv[i]);
    }

    for (const auto& cmd : commands) {
        if (cmd->name() == arg1) {
            return cmd->execute(repo_root, cmd_args);
        }
    }

    Terminal::fail("Unknown Command", "No command named '" + arg1 + "'. Run 'enki --help' for available commands.");
    return 1;
}
