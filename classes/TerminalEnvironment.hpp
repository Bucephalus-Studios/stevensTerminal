#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace stevensTerminal
{
    enum class TerminalType
    {
        WindowsTerminal,
        WindowsConsole,
        Xterm,
        GnomeTerminal,
        Vte,
        Konsole,
        Kitty,
        Alacritty,
        WezTerm,
        LinuxConsole,
        Unknown
    };

    /**
     * @brief Everything stevensTerminal knows about one kind of terminal: how to recognize it, whether it
     *        can report its font, and neutral how-to text for changing its window size, font, and colors.
     *        Prototypes live in the TerminalEnvironments registry -- get them through
     *        TerminalEnvironments::detect() or TerminalEnvironments::get() rather than constructing one.
     */
    class TerminalEnvironment
    {
    public:
        TerminalType type = TerminalType::Unknown;
        std::string displayName;
        /** @brief Environment variables this terminal sets for its child processes; any one being set matches. */
        std::vector<std::string> identifyingEnvVars;
        /** @brief Extra match for terminals without a unique variable (e.g. TERM=linux). Optional. */
        std::function<bool()> extraMatch;
        std::string windowSizeTip;
        std::string fontTip;
        std::string colorsTip;
        /** @brief Asks the running terminal for its font. Empty when the terminal can't report it. */
        std::function<std::optional<std::string>()> fontQuery;

        const std::string & getDisplayName() const { return displayName; }
        const std::string & getWindowSizeTip() const { return windowSizeTip; }
        const std::string & getFontTip() const { return fontTip; }
        const std::string & getColorsTip() const { return colorsTip; }

        bool canReportFont() const { return static_cast<bool>(fontQuery); }

        /** @brief The terminal's current font formatted for display, or std::nullopt if it can't be determined. */
        std::optional<std::string> queryFontName() const
        {
            if (!fontQuery)
            {
                return std::nullopt;
            }
            return fontQuery();
        }

        /** @brief True when the current process appears to be running inside this terminal. */
        bool matchesCurrentEnvironment() const;
    };
}
