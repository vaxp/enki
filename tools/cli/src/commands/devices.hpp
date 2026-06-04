#pragma once
/// @file devices.hpp
/// @brief 'enki devices' command: lists connected Android devices and Desktop targets.

#include "commands/command.hpp"

namespace enki::cli {

class DevicesCommand : public Command {
public:
    std::string name() const override { return "devices"; }
    std::string description() const override { return "List available target devices (Desktop, Emulators, Physical phones)"; }
    int execute(const fs::path& repo_root, const std::vector<std::string>& args) override;
};

} // namespace enki::cli
