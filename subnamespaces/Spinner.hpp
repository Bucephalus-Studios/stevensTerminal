#pragma once
/**
 * @file Spinner.hpp
 * @brief Animated spinner (loading indicator) utilities for stevensTerminal.
 */

#include <string>
#include <vector>
#include "Colors.hpp"

#if defined(__linux__)
    #include <ncurses.h>
#elif defined(_WIN32) || defined(__MSDOS__)
    #include <curses.h>
#endif

namespace stevensTerminal
{
    /**
     * @brief Configuration for a printSpinner call.
     *
     * frames    — UTF-8 strings cycled in order; defaults to a rotating
     *             small-triangle set. Was the quarter-circle glyphs
     *             (U+25D0-25D3, Geometric Shapes block) - not covered by
     *             Consolas; glyph::spinnerUp/Right/Down/Left (U+25B4/25B8/
     *             25BE/25C2, same block) are.
     * fgColor   — ncurses color name for the spinner character.
     * bgColor   — ncurses color name for the background.
     * bold      — if true, A_BOLD is applied.
     */
    struct SpinnerSpec
    {
        std::vector<std::string> frames   = { std::string(glyph::spinnerUp),
                                               std::string(glyph::spinnerRight),
                                               std::string(glyph::spinnerDown),
                                               std::string(glyph::spinnerLeft) };
        std::string              fgColor  = "bright-yellow";
        std::string              bgColor  = "black";
        bool                     bold     = true;
    };

    /**
     * @brief Render one frame of a spinner animation into a curses window.
     *
     * Advance `frame` by 1 each tick (e.g. via an atomic counter in an animation
     * thread) and call this on the main thread to show the current frame.
     *
     * @param win    The curses window to render into.
     * @param y      Row in the window (0-based).
     * @param x      Column in the window (0-based).
     * @param frame  Monotonically increasing tick counter; modulo'd against the frame count.
     * @param spec   Visual options (frame set, colors, bold).
     */
    inline void printSpinner(WINDOW*            win,
                             int                y,
                             int                x,
                             int                frame,
                             const SpinnerSpec& spec = {})
    {
        if (!win || spec.frames.empty()) return;

        const std::string & ch = spec.frames[frame % static_cast<int>(spec.frames.size())];
        int pair = Colors::lookupColorPairByName(spec.fgColor, spec.bgColor);
        chtype attrs = COLOR_PAIR(pair);
        if (spec.bold) attrs |= A_BOLD;

        wattron(win, attrs);
        mvwaddstr(win, y, x, ch.c_str());
        wattroff(win, attrs);
    }

    /** Ready-made SpinnerSpec presets. Pass one as the last arg to printSpinner(). */
    namespace spinners
    {
        inline const SpinnerSpec pipe    {{ "|", "/", "-", "\\" }};
        inline const SpinnerSpec dots    {{ ".", "..", "..." }};
        inline const SpinnerSpec braille {{ "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" }};
        inline const SpinnerSpec triangle{{ std::string(glyph::spinnerUp),
                                           std::string(glyph::spinnerRight),
                                           std::string(glyph::spinnerDown),
                                           std::string(glyph::spinnerLeft) }};
    }

} // namespace stevensTerminal
