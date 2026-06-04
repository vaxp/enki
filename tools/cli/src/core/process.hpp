#pragma once
/// @file process.hpp
/// @brief Cross-platform asynchronous and synchronous child process executor with piped output.

#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace enki::cli {

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_str;
    std::string stderr_str;
    bool success() const { return exit_code == 0; }
};

class Process {
public:
    using LineCallback = std::function<void(const std::string& line)>;

    /// @brief Run command synchronously and collect or stream outputs.
    static ProcessResult run(
        const std::string& command_line,
        const std::string& working_dir = "",
        bool stream_output = false,
        LineCallback on_stdout_line = nullptr,
        const std::vector<std::pair<std::string, std::string>>& extra_env = {}
    );

    /// @brief Run command detached or in background.
    static bool spawnDetached(
        const std::string& command_line,
        const std::string& working_dir = ""
    );
};

} // namespace enki::cli
