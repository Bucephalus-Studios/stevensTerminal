#pragma once
/**
 * @file Bar.hpp
 * @brief Progress bar and indicator utilities for stevensTerminal
 */

#include <string>
#include <string_view>
#include <algorithm>
#include "Colors.hpp"

#if defined(__linux__)
    #include <ncurses.h>
#elif defined(_WIN32) || defined(__MSDOS__)
    #include <curses.h>
#endif

namespace stevensTerminal
{
    /**
     * @brief Spec for a printBar/createBarString call.
     *
     * fillBgColor / fillFgColor   — ncurses color names for the completed portion
     * emptyBgColor / emptyFgColor — ncurses color names for the remaining portion
     * width                       — total character width of the bar
     * fillChar                    — character(s) for filled cells; defaults to " " (space/bg-color bar)
     * halfChar                    — character(s) shown at the sub-cell boundary when fractional fill >= 0.5
     *                               (only used when fillChar != " " and smoothPartial is false);
     *                               leave "" to skip half-cell rendering
     * smoothPartial               — when true (and using block chars), the boundary cell is filled
     *                               to the exact fraction using left eighth-blocks (▏▎▍▌▋▊▉) instead
     *                               of the single halfChar step. Overrides halfChar when set.
     * emptyChar                   — character(s) for empty cells; defaults to " "
     * showPct                     — overlay "45%" text centered on the bar (only applies when using space chars)
     * showFraction                — overlay "6/10" (current/total) text centered on the bar instead of a
     *                               percentage (only applies when using space chars; takes priority over showPct)
     */
    struct BarSpec
    {
        int         width         = 40;
        std::string fillBgColor   = "green";
        std::string fillFgColor   = "black";
        std::string emptyBgColor  = "black";
        std::string emptyFgColor  = "white";
        std::string fillChar      = " ";
        std::string halfChar      = "";
        bool        smoothPartial = false;
        std::string emptyChar     = " ";
        bool        showPct       = true;
        bool        showFraction  = false;
    };

    /**
     * @brief Builds a bar as a plain string of $[textColor=...,bgColor=...]-tokenized segments,
     *        suitable for embedding in any text that flows through the style-tokenizing print
     *        pipeline (e.g. cg_response.text), rather than drawn directly into a curses window.
     *
     * Mirrors printBar()'s two rendering modes (see BarSpec doc comment): the space/bg-color
     * mode with a centered pct-or-fraction text overlay, and the block-character mode with
     * sub-cell precision. printBar() is a thin wrapper around this function.
     *
     * @param current Current value
     * @param total   Maximum value
     * @param spec    Visual options (colors, width, characters)
     * @return Tokenized bar string, or "" if total <= 0
     */
    inline std::string createBarString(int current, int total, const BarSpec & spec = {})
    {
        if (total <= 0) { return ""; }

        int w = spec.width;
        bool useBlockChars = (spec.fillChar != " " || spec.emptyChar != " ");
        std::string result;

        if (!useBlockChars)
        {
            // Space/bg-color bar with an optional centered pct-or-fraction text overlay
            int filled = std::clamp((int)((float)current / total * w), 0, w);

            std::string overlay;
            if (spec.showFraction)      { overlay = std::to_string(current) + "/" + std::to_string(total); }
            else if (spec.showPct)      { overlay = std::to_string(std::clamp((int)(100.0f * current / total), 0, 100)) + "%"; }
            int overlayStart = std::max(0, (w - (int)overlay.size()) / 2);

            int i = 0;
            while (i < w)
            {
                bool isFilled = i < filled;
                std::string bg = isFilled ? spec.fillBgColor : spec.emptyBgColor;
                std::string fg = isFilled ? spec.fillFgColor : spec.emptyFgColor;

                std::string segment;
                int j = i;
                while (j < w && (j < filled) == isFilled)
                {
                    bool inOverlay = !overlay.empty() && j >= overlayStart && j < overlayStart + (int)overlay.size();
                    segment += inOverlay ? overlay[j - overlayStart] : ' ';
                    ++j;
                }
                result += "{" + segment + "}$[textColor=" + fg + ",bgColor=" + bg + "]";
                i = j;
            }
        }
        else
        {
            // Block-character bar with sub-cell precision (halfChar step, or smooth eighths).
            float fillF     = std::clamp((float)current / total * w, 0.0f, (float)w);
            int   fullCells = (int)fillF;
            float frac      = fillF - (float)fullCells;

            // Determine the partial (boundary) cell glyph, if any.
            std::string partialChar;
            if (spec.smoothPartial)
            {
                // Left partial blocks (glyph::) fill the boundary cell to the exact eighth.
                // Index 0 = none, 8 = a full cell (promoted below).
                static const std::string_view EIGHTHS[9] = {
                    "",
                    glyph::leftOneEighthBlock,
                    glyph::leftOneQuarterBlock,
                    glyph::leftThreeEighthsBlock,
                    glyph::leftHalfBlock,
                    glyph::leftFiveEighthsBlock,
                    glyph::leftThreeQuartersBlock,
                    glyph::leftSevenEighthsBlock,
                    ""
                };
                int eighth = (int)(frac * 8.0f + 0.5f); // 0..8
                if (eighth >= 8 && fullCells < w)      { ++fullCells; }                     // rounds up to a full cell
                else if (eighth > 0 && fullCells < w)  { partialChar = std::string(EIGHTHS[eighth]); }
            }
            else if (!spec.halfChar.empty() && frac >= 0.5f && fullCells < w)
            {
                partialChar = spec.halfChar;
            }

            int emptyStart = fullCells + (partialChar.empty() ? 0 : 1);
            int emptyCount = w - emptyStart;

            if (fullCells > 0)
            {
                std::string fill;
                for (int i = 0; i < fullCells; ++i) fill += spec.fillChar;
                result += "{" + fill + "}$[textColor=" + spec.fillFgColor + ",bgColor=" + spec.fillBgColor + "]";
            }

            if (!partialChar.empty())
            {
                // Boundary cell: fill foreground (the "ink") against the empty background.
                result += "{" + partialChar + "}$[textColor=" + spec.fillFgColor + ",bgColor=" + spec.emptyBgColor + "]";
            }

            if (emptyCount > 0)
            {
                std::string empty;
                for (int i = 0; i < emptyCount; ++i) empty += spec.emptyChar;
                result += "{" + empty + "}$[textColor=" + spec.emptyFgColor + ",bgColor=" + spec.emptyBgColor + "]";
            }
        }

        return result;
    }

    /**
     * @brief Render a horizontal bar directly into a curses window.
     *
     * Thin wrapper around createBarString(): builds the tokenized bar string, then prints it
     * through the standard style-tokenizing print pipeline so both draw paths (direct-to-window
     * here, or embedded in response text via createBarString() alone) share one layout source.
     *
     * @param win     The curses window to render into
     * @param y       Row in the window (0-based)
     * @param x       Column offset in the window (0-based)
     * @param current Current value
     * @param total   Maximum value
     * @param spec    Visual options (colors, width, characters)
     */
    inline void printBar(WINDOW*          win,
                         int              y,
                         int              x,
                         int              current,
                         int              total,
                         const BarSpec&   spec = {})
    {
        if (!win || total <= 0) return;

        std::string barString = createBarString(current, total, spec);
        PrintHelper::curses_wprint(win, y, x, barString, {}, {}, textStyling);
    }

} // namespace stevensTerminal
