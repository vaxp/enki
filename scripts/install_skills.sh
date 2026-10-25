#!/usr/bin/env bash
# ==============================================================================
# ENKI Framework — AI Agent Skills Installer
# ==============================================================================
# Installs the comprehensive ENKI AI Skill into your AI coding assistants
# (Google Antigravity IDE, Claude Code, Cursor, Windsurf, Copilot, etc.)
# so that any AI tool can build and maintain ENKI applications without
# inspecting the C++ source code.
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENKI_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SKILL_SRC="$ENKI_ROOT/skills/enki-framework"

# ANSI Colors
BOLD="\033[1m"
GREEN="\033[32m"
CYAN="\033[36m"
YELLOW="\033[33m"
RESET="\033[0m"

echo -e "${BOLD}${CYAN}======================================================${RESET}"
echo -e "${BOLD}${CYAN}      ENKI Framework — AI Agent Skills Installer      ${RESET}"
echo -e "${BOLD}${CYAN}======================================================${RESET}"
echo ""

if [ ! -d "$SKILL_SRC" ]; then
    echo -e "${YELLOW}[!] Skill source not found at: $SKILL_SRC${RESET}"
    exit 1
fi

install_to() {
    local target_dir="$1"
    local name="$2"
    echo -e "Installing to ${BOLD}$name${RESET} at: ${CYAN}$target_dir${RESET}..."
    mkdir -p "$target_dir"
    cp -r "$SKILL_SRC"/* "$target_dir/"
    echo -e "${GREEN}  ✓ Done! ($target_dir)${RESET}"
}

# 1. Antigravity IDE (Global)
ANTIGRAVITY_GLOBAL="$HOME/.gemini/config/skills/enki-framework"
install_to "$ANTIGRAVITY_GLOBAL" "Antigravity IDE (Global Config)"

# 2. Antigravity IDE (Local Workspace)
ANTIGRAVITY_LOCAL="$ENKI_ROOT/.agents/skills/enki-framework"
if [ "$SKILL_SRC" != "$ANTIGRAVITY_LOCAL" ]; then
    install_to "$ANTIGRAVITY_LOCAL" "Antigravity IDE (Workspace)"
fi

# 3. Claude Code (Global, if ~/.claude exists)
if [ -d "$HOME/.claude" ]; then
    CLAUDE_GLOBAL="$HOME/.claude/skills/enki-framework"
    install_to "$CLAUDE_GLOBAL" "Claude Code (Global)"
fi

# 4. Optional Custom Target via Argument
if [ -n "$1" ]; then
    CUSTOM_TARGET="$1/enki-framework"
    install_to "$CUSTOM_TARGET" "Custom Target Directory"
fi

echo ""
echo -e "${BOLD}${GREEN}Installation Complete!${RESET}"
echo -e "AI Agents will now automatically recognize ENKI and use its declarative"
echo -e "widgets, reactive Cubit state management, and lifecycle patterns with"
echo -e "sub-millisecond precision without needing to inspect the C++ source code."
