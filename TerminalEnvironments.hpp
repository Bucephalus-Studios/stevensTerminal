#pragma once

#include "classes/TerminalEnvironment.hpp"

/**
 * @brief Registry of TerminalEnvironment prototypes, one per supported terminal.
 *
 *        Detection is best-effort: terminals identify themselves through environment variables, and a
 *        terminal started from inside another terminal inherits the parent's, so nested terminals can be
 *        misidentified. detect() always returns a usable entry -- the Unknown entry, with generic tips,
 *        when nothing matches.
 *
 *        Standalone header: include it directly rather than through stevensTerminal.hpp. Font queries
 *        and getColorCount() require curses to be initialized first (stevensTerminal::initialize()).
 */
namespace stevensTerminal::TerminalEnvironments
{
    /** @brief The terminal the program is currently running in, or the Unknown entry. */
    const TerminalEnvironment & detect();

    /** @brief The registry entry for a terminal type; the Unknown entry for types not supported on this platform. */
    const TerminalEnvironment & get(TerminalType type);

    /** @brief Number of colors curses reports for this terminal, or 0 if it has no color support. */
    int getColorCount();
}
