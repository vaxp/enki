#pragma once
/// @file command.hpp
/// @brief Base interface for ENKI CLI commands.

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace enki::cli {

class Command {
public:
    virtual ~Command() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string description() const = 0;
    virtual int execute(const fs::path& repo_root, const std::vector<std::string>& args) = 0;
};

} // namespace enki::cli
