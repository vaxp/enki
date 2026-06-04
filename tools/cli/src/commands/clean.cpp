#include "commands/clean.hpp"
#include "core/terminal.hpp"
#include "core/process.hpp"
#include "core/env.hpp"

namespace enki::cli {

int CleanCommand::execute(const fs::path& repo_root, const std::vector<std::string>&) {
    Terminal::header("Cleaning Build Artifacts & Caches");

    std::vector<std::string> dirs = { "build-Win", "build-android" };
    for (const auto& d : dirs) {
        fs::path p = repo_root / d;
        std::error_code ec;
        if (fs::exists(p, ec)) {
            std::string ninja_bin = "ninja";
            if (auto np = Env::findNinja()) ninja_bin = Env::pathToUtf8(*np);
            std::string ninja_clean = "\"" + ninja_bin + "\" -C \"" + Env::pathToUtf8(p) + "\" clean";
            Process::run(ninja_clean);
            Terminal::ok("Cleaned build target", d);
        }
    }

    Terminal::ok("Clean Complete", "All intermediate targets cleaned.");
    return 0;
}

} // namespace enki::cli
