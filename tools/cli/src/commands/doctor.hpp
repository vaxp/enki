#pragma once
/// @file doctor.hpp
/// @brief 'enki doctor' command: comprehensive environment and toolchain diagnosis.

#include "commands/command.hpp"

namespace enki::cli {

class DoctorCommand : public Command {
public:
    std::string name() const override { return "doctor"; }
    std::string description() const override { return "Check the development environment, compilers, SDKs, and devices"; }
    int execute(const fs::path& repo_root, const std::vector<std::string>& args) override;
};

} // namespace enki::cli
