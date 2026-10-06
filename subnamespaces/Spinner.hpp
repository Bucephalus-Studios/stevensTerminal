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
     *             Consolas; glyph::smallTriangleUp/Right/Down/Left (U+25B4/25B8/
     *             25BE/25C2, same block) are.
     * fgColor   — ncurses color name for the spinner character.
     * bgColor   — ncurses color name for the background.
     * bold      — if true, A_BOLD is applied.
     */
    struct SpinnerSpec
    {
        std::vector<std::string> frames   = { std::string(glyph::smallTriangleUp),
                                               std::string(glyph::smallTriangleRight),
                                               std::string(glyph::smallTriangleDown),
                                               std::string(glyph::smallTriangleLeft) };
        std::string              fgColor  = "bright-yellow";
        std::string              bgColor  = "black";
        bool                     bold     = true;
    };

    /**
     * @brief A spinner animation over a discrete set of frames, stepped by its owner.
     *
     * Call advance() once per redraw of whatever loop owns the curses screen, then read
     * currentFrame() to draw it -- so the animation speed is that loop's redraw rate. Holds no
     * curses state and never draws on its own (curses is not thread-safe; drawing stays with the
     * single loop that owns the screen), and needs no animation thread or shared counter.
     */
    class Spinner
    {
    public:
        explicit Spinner(const SpinnerSpec & spec = {}) : spec(spec) {}

        /** @brief Step to the next frame, wrapping back to the first after the last. */
        void advance()
        {
            if (spec.frames.empty())
            {
                return;
            }
            frameIndex = (frameIndex + 1) % static_cast<int>(spec.frames.size());
        }

        /** @brief The glyph for the current frame (empty string if the spec has no frames). */
        const std::string & currentFrame() const
        {
            static const std::string NO_FRAME;
            if (spec.frames.empty())
            {
                return NO_FRAME;
            }
            return spec.frames[frameIndex];
        }

        const SpinnerSpec & getSpec() const { return spec; }

    private:
        SpinnerSpec spec;
        int frameIndex = 0;
    };

    /**
     * @brief Render one frame of a spinner animation into a curses window.
     *
     * Advance `frame` by 1 each redraw of the loop that owns the curses screen (a Spinner
     * object does this bookkeeping for you), and call this from that same loop -- curses is not
     * thread-safe, so drawing belongs to that loop, and no separate animation thread is needed.
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
        inline const SpinnerSpec triangle{{ std::string(glyph::smallTriangleUp),
                                           std::string(glyph::smallTriangleRight),
                                           std::string(glyph::smallTriangleDown),
                                           std::string(glyph::smallTriangleLeft) }};
    }

} // namespace stevensTerminal
