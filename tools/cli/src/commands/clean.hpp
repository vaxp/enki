#pragma once
/// @file clean.hpp
/// @brief 'enki clean' command: cleans build directories and cache.

#include "commands/command.hpp"

namespace enki::cli {

class CleanCommand : public Command {
public:
    std::string name() const override { return "clean"; }
    std::string description() const override { return "Clean build artifacts and temporary caches"; }
    int execute(const fs::path& repo_root, const std::vector<std::string>& args) override;
};

} // namespace enki::cli
