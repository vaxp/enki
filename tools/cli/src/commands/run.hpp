#pragma once
/// @file run.hpp
/// @brief 'enki run' command: compiles, packages, launches, and streams logs.

#include "commands/command.hpp"

namespace enki::cli {

class RunCommand : public Command {
public:
    std::string name() const override { return "run"; }
    std::string description() const override { return "Build, deploy, and launch an application with live logs"; }
    int execute(const fs::path& repo_root, const std::vector<std::string>& args) override;
};

} // namespace enki::cli
