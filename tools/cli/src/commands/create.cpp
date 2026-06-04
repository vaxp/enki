#include "commands/create.hpp"
#include "scaffolding/project_template.hpp"
#include "core/terminal.hpp"

namespace enki::cli {

int CreateCommand::execute(const fs::path& repo_root, const std::vector<std::string>& args) {
    if (args.empty()) {
        Terminal::fail("Missing Argument", "Usage: enki create <application_name>");
        return 1;
    }

    std::string app_name = args[0];
    if (ProjectTemplate::create(repo_root, app_name)) {
        return 0;
    }
    return 1;
}

} // namespace enki::cli
