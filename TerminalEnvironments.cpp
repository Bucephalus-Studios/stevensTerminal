#include "TerminalEnvironments.hpp"

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    // wincon.h and PDCurses both define MOUSE_MOVED; PDCurses' definition is the one curses code expects.
    #undef MOUSE_MOVED
    #include <curses.h>
#else
    #include <ncurses.h>
    #include <poll.h>
    #include <unistd.h>
#endif

#include <cctype>
#include <chrono>
#include <cstdlib>

namespace stevensTerminal
{

bool TerminalEnvironment::matchesCurrentEnvironment() const
{
    for (const std::string & name : identifyingEnvVars)
    {
        const char * value = std::getenv(name.c_str());
        if (value != nullptr && value[0] != '\0')
        {
            return true;
        }
    }
    return extraMatch && extraMatch();
}

namespace TerminalEnvironments
{
namespace
{

#if defined(_WIN32)

    std::optional<std::string> queryWindowsConsoleFont()
    {
        CONSOLE_FONT_INFOEX fontInfo{};
        fontInfo.cbSize = sizeof(fontInfo);
        if (!GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &fontInfo))
        {
            return std::nullopt;
        }
        int byteCount = WideCharToMultiByte(CP_UTF8, 0, fontInfo.FaceName, -1, nullptr, 0, nullptr, nullptr);
        if (byteCount <= 1)
        {
            return std::nullopt;
        }
        std::string faceName(static_cast<size_t>(byteCount - 1), '\0');
        WideCharToMultiByte(CP_UTF8, 0, fontInfo.FaceName, -1, faceName.data(), byteCount, nullptr, nullptr);
        return faceName + " (" + std::to_string(fontInfo.dwFontSize.Y) + "px)";
    }

    TerminalEnvironment getWindowsTerminalEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::WindowsTerminal;
        environment.displayName = "Windows Terminal";
        environment.identifyingEnvVars = {"WT_SESSION"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Windows Terminal ignores programs that ask to resize it, "
            "so this has to be done by hand. To always open at a larger size, open Settings (Ctrl + comma), go to "
            "Startup, and raise the Columns and Rows under Launch size. Making the font smaller with Ctrl + minus "
            "also fits more on screen.";
        environment.fontTip =
            "Open Settings (Ctrl + comma), select your profile under Profiles, open Appearance, and change the "
            "Font face. Ctrl + plus and Ctrl + minus change the font size, and Ctrl + 0 resets it.";
        environment.colorsTip =
            "Open Settings (Ctrl + comma), select your profile under Profiles, open Appearance, and pick a "
            "different Color scheme.";
        // No fontQuery: under Windows Terminal the console API reports ConPTY's defaults, not the real font.
        return environment;
    }

    TerminalEnvironment getWindowsConsoleEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::WindowsConsole;
        environment.displayName = "Windows Console";
        // Any Windows session that isn't Windows Terminal is the classic console host.
        environment.extraMatch = []() { return true; };
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. You can also right-click the title bar, choose "
            "Properties, and raise the Width and Height under Window Size on the Layout tab. Holding Ctrl and "
            "scrolling the mouse wheel changes the font size.";
        environment.fontTip =
            "Right-click the title bar, choose Properties, and pick a font and size on the Font tab. Holding Ctrl "
            "and scrolling the mouse wheel also changes the font size.";
        environment.colorsTip =
            "Right-click the title bar, choose Properties, and adjust the colors on the Colors tab.";
        environment.fontQuery = queryWindowsConsoleFont;
        return environment;
    }

#else

    constexpr int FONT_QUERY_TIMEOUT_MS = 300;

    std::string getEnv(const char * name)
    {
        const char * value = std::getenv(name);
        return value != nullptr ? value : "";
    }

    /**
     * @brief Turns an X logical font description ("-misc-fixed-medium-r-semicondensed--13-...") into
     *        "Fixed (13px)". Any other font name is returned as-is, minus an "xft:" prefix.
     */
    std::string formatFontNameForDisplay(const std::string & rawName)
    {
        if (rawName.rfind("xft:", 0) == 0)
        {
            return rawName.substr(4);
        }
        if (rawName.empty() || rawName[0] != '-')
        {
            return rawName;
        }

        std::vector<std::string> fields;
        std::string field;
        for (char c : rawName.substr(1))
        {
            if (c == '-')
            {
                fields.push_back(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }
        fields.push_back(field);

        // XLFD fields after the leading '-': foundry, family, weight, slant, setwidth, addstyle, pixel size, ...
        if (fields.size() < 7 || fields[1].empty())
        {
            return rawName;
        }
        std::string family = fields[1];
        family[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(family[0])));
        const std::string & pixelSize = fields[6];
        if (pixelSize.empty() || pixelSize == "*" || pixelSize == "0")
        {
            return family;
        }
        return family + " (" + pixelSize + "px)";
    }

    /**
     * @brief Sends xterm's font query (OSC 50 ; ?) straight to the tty and reads the reply without going
     *        through curses. xterm only replies when its allowFontOps resource is on; otherwise the query
     *        is silently ignored, so after one timeout we stop asking for the rest of the session.
     */
    std::optional<std::string> queryXtermFont()
    {
        static bool xtermIgnoresFontQueries = false;
        if (xtermIgnoresFontQueries)
        {
            return std::nullopt;
        }

        constexpr char QUERY[] = "\033]50;?\007";
        if (write(STDOUT_FILENO, QUERY, sizeof(QUERY) - 1) < 0)
        {
            return std::nullopt;
        }

        std::string reply;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(FONT_QUERY_TIMEOUT_MS);
        while (true)
        {
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now()).count();
            if (remaining <= 0)
            {
                break;
            }
            pollfd input{STDIN_FILENO, POLLIN, 0};
            if (poll(&input, 1, static_cast<int>(remaining)) <= 0)
            {
                break;
            }
            char c = 0;
            if (read(STDIN_FILENO, &c, 1) != 1)
            {
                break;
            }
            reply += c;
            if (c == '\007' || reply.ends_with("\033\\"))
            {
                break;
            }
        }
        // Anything left over (a partial reply, or keys typed during the wait) would otherwise reach
        // curses as keypresses -- and a font name is full of digits that could select menu options.
        flushinp();

        const std::string REPLY_PREFIX = "\033]50;";
        const size_t prefixStart = reply.find(REPLY_PREFIX);
        if (prefixStart == std::string::npos)
        {
            xtermIgnoresFontQueries = true;
            return std::nullopt;
        }
        std::string fontName = reply.substr(prefixStart + REPLY_PREFIX.size());
        if (fontName.ends_with("\007"))
        {
            fontName.pop_back();
        }
        else if (fontName.ends_with("\033\\"))
        {
            fontName.resize(fontName.size() - 2);
        }
        if (fontName.empty())
        {
            return std::nullopt;
        }
        return formatFontNameForDisplay(fontName);
    }

    TerminalEnvironment getKittyEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Kitty;
        environment.displayName = "kitty";
        environment.identifyingEnvVars = {"KITTY_WINDOW_ID"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Ctrl + Shift + equals and Ctrl + Shift + minus change "
            "the font size. To always open at a larger size, set initial_window_width and initial_window_height "
            "in kitty.conf (a value like 82c means 82 cells).";
        environment.fontTip =
            "Set font_family and font_size in kitty.conf. Ctrl + Shift + equals and Ctrl + Shift + minus change the "
            "size for the current window.";
        environment.colorsTip =
            "kitty's themes kitten lets you browse and apply color themes, or you can set the colors directly in "
            "kitty.conf.";
        return environment;
    }

    TerminalEnvironment getAlacrittyEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Alacritty;
        environment.displayName = "Alacritty";
        environment.identifyingEnvVars = {"ALACRITTY_WINDOW_ID", "ALACRITTY_SOCKET", "ALACRITTY_LOG"};
        environment.extraMatch = []() { return getEnv("TERM") == "alacritty"; };
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Ctrl + plus and Ctrl + minus change the font size. To "
            "always open at a larger size, set the columns and lines under [window.dimensions] in alacritty.toml.";
        environment.fontTip =
            "Set the font family under [font.normal] and the size under [font] in alacritty.toml.";
        environment.colorsTip =
            "Set the colors under [colors] in alacritty.toml, or import one of the many community color themes.";
        return environment;
    }

    TerminalEnvironment getWezTermEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::WezTerm;
        environment.displayName = "WezTerm";
        environment.extraMatch = []() { return getEnv("TERM_PROGRAM") == "WezTerm"; };
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Ctrl + plus and Ctrl + minus change the font size. To "
            "always open at a larger size, set initial_cols and initial_rows in wezterm.lua.";
        environment.fontTip =
            "Set config.font and config.font_size in wezterm.lua.";
        environment.colorsTip =
            "Set config.color_scheme in wezterm.lua to any of WezTerm's built-in color schemes.";
        return environment;
    }

    TerminalEnvironment getKonsoleEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Konsole;
        environment.displayName = "Konsole";
        environment.identifyingEnvVars = {"KONSOLE_VERSION"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Ctrl + plus and Ctrl + minus change the font size. To "
            "always open at a larger size, look for the terminal size setting in your profile (Settings, then "
            "Edit Current Profile).";
        environment.fontTip =
            "Open Settings, then Edit Current Profile, go to Appearance, and choose a font.";
        environment.colorsTip =
            "Open Settings, then Edit Current Profile, go to Appearance, and pick a color scheme.";
        return environment;
    }

    TerminalEnvironment getXtermEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Xterm;
        environment.displayName = "xterm";
        environment.identifyingEnvVars = {"XTERM_VERSION"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. To change the font size, hold Shift and press plus or "
            "minus on the number pad, or use the VT Fonts menu: hold Ctrl, press and keep holding the right mouse "
            "button inside the window, then drag to a size and release. The menu closes as soon as you let go of "
            "the mouse button.";
        environment.fontTip =
            "Open the VT Fonts menu by holding Ctrl and pressing and keeping hold of the right mouse button inside "
            "the window. Drag to a size or to the TrueType fonts option and release to choose it; the menu closes "
            "as soon as you let go. xterm has no full font picker; advanced users can set XTerm*faceName in "
            "~/.Xresources.";
        environment.colorsTip =
            "xterm has no color theme menu. Advanced users can set XTerm*color0 through XTerm*color15 in "
            "~/.Xresources.";
        environment.fontQuery = queryXtermFont;
        return environment;
    }

    TerminalEnvironment getGnomeTerminalEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::GnomeTerminal;
        environment.displayName = "GNOME Terminal";
        environment.identifyingEnvVars = {"GNOME_TERMINAL_SCREEN", "GNOME_TERMINAL_SERVICE"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Ctrl + plus and Ctrl + minus change the font size, and "
            "Ctrl + 0 resets it. To always open at a larger size, open the menu (the button with three lines), "
            "choose Preferences, select your profile, and raise the initial terminal size on the Text tab.";
        environment.fontTip =
            "Open the menu (the button with three lines), choose Preferences, select your profile, and turn on "
            "Custom font on the Text tab. This changes the font for everything you run in GNOME Terminal.";
        environment.colorsTip =
            "Open the menu (the button with three lines), choose Preferences, select your profile, and pick a "
            "built-in scheme or palette on the Colors tab.";
        return environment;
    }

    TerminalEnvironment getVteEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Vte;
        environment.displayName = "a VTE-based terminal";
        environment.identifyingEnvVars = {"VTE_VERSION"};
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Most terminals like this one change the font size with "
            "Ctrl + plus and Ctrl + minus, and set a starting window size in their Preferences.";
        environment.fontTip =
            "Look for Preferences or a profile setting in this terminal's menu to choose a font.";
        environment.colorsTip =
            "Look for Preferences or a profile setting in this terminal's menu to choose a color scheme.";
        return environment;
    }

    TerminalEnvironment getLinuxConsoleEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::LinuxConsole;
        environment.displayName = "the Linux console";
        environment.extraMatch = []() { return getEnv("TERM") == "linux"; };
        environment.windowSizeTip =
            "The Linux console's size comes from your screen resolution and console font, so a smaller console "
            "font fits more text. On Debian and Ubuntu you can change it by running "
            "sudo dpkg-reconfigure console-setup after exiting.";
        environment.fontTip =
            "On Debian and Ubuntu you can change the console font by running sudo dpkg-reconfigure console-setup "
            "after exiting.";
        environment.colorsTip =
            "The Linux console supports a limited set of colors and has no theme settings. A graphical terminal "
            "gives you many more options.";
        return environment;
    }

#endif

    TerminalEnvironment getUnknownEnvironment()
    {
        TerminalEnvironment environment;
        environment.type = TerminalType::Unknown;
        environment.displayName = "an unrecognized terminal";
        environment.windowSizeTip =
            "Drag the edges of this window or maximize it. Most terminals change the font size with Ctrl + plus "
            "and Ctrl + minus, or by holding Ctrl and scrolling the mouse wheel, and set a starting window size "
            "in their Preferences or Settings.";
        environment.fontTip =
            "Look for Preferences, Settings, or a profile option in your terminal's menu to choose a font.";
        environment.colorsTip =
            "Look for Preferences, Settings, or a profile option in your terminal's menu to choose a color scheme.";
        return environment;
    }

    /**
     * @brief Builds the registry in detection order. A terminal started from inside another inherits the
     *        parent's variables, so terminals with their own variable are checked before generic ones
     *        (xterm before GNOME Terminal before any VTE terminal). Unknown is always last.
     */
    std::vector<TerminalEnvironment> load()
    {
    #if defined(_WIN32)
        return {
            getWindowsTerminalEnvironment(),
            getWindowsConsoleEnvironment(),
            getUnknownEnvironment()
        };
    #else
        return {
            getKittyEnvironment(),
            getAlacrittyEnvironment(),
            getWezTermEnvironment(),
            getKonsoleEnvironment(),
            getXtermEnvironment(),
            getGnomeTerminalEnvironment(),
            getVteEnvironment(),
            getLinuxConsoleEnvironment(),
            getUnknownEnvironment()
        };
    #endif
    }

    const std::vector<TerminalEnvironment> & registry()
    {
        static const std::vector<TerminalEnvironment> environments = load();
        return environments;
    }

} // anonymous namespace


const TerminalEnvironment & detect()
{
    for (const TerminalEnvironment & environment : registry())
    {
        if (environment.matchesCurrentEnvironment())
        {
            return environment;
        }
    }
    return get(TerminalType::Unknown);
}

const TerminalEnvironment & get(TerminalType type)
{
    for (const TerminalEnvironment & environment : registry())
    {
        if (environment.type == type)
        {
            return environment;
        }
    }
    return registry().back();
}

int getColorCount()
{
    return has_colors() ? COLORS : 0;
}

} // namespace TerminalEnvironments
} // namespace stevensTerminal
