#pragma once
/// @file create.hpp
/// @brief 'enki create' command: scaffolds a new ENKI application.

#include "commands/command.hpp"

namespace enki::cli {

class CreateCommand : public Command {
public:
    std::string name() const override { return "create"; }
    std::string description() const override { return "Scaffold a new ENKI application with clean architecture"; }
    int execute(const fs::path& repo_root, const std::vector<std::string>& args) override;
};

} // namespace enki::cli
