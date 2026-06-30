#pragma once
/**
 * @file Bar.hpp
 * @brief Progress bar and indicator utilities for stevensTerminal
 */

#include <string>
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
     * @brief Spec for a printBar call.
     *
     * fillBgColor / fillFgColor   — ncurses color names for the completed portion
     * emptyBgColor / emptyFgColor — ncurses color names for the remaining portion
     * width                       — total character width of the bar
     * fillChar                    — character(s) for filled cells; defaults to " " (space/bg-color bar)
     * halfChar                    — character(s) shown at the sub-cell boundary when fractional fill >= 0.5
     *                               (only used when fillChar != " "); leave "" to skip half-cell rendering
     * emptyChar                   — character(s) for empty cells; defaults to " "
     * showPct                     — overlay "45%" text centered on the bar (only applies when using space chars)
     */
    struct BarSpec
    {
        int         width        = 40;
        std::string fillBgColor  = "green";
        std::string fillFgColor  = "black";
        std::string emptyBgColor = "black";
        std::string emptyFgColor = "white";
        std::string fillChar     = " ";
        std::string halfChar     = "";
        std::string emptyChar    = " ";
        bool        showPct      = true;
    };

    /**
     * @brief Render a horizontal bar directly into a curses window.
     *
     * When fillChar / emptyChar are spaces (the default), the bar is drawn using
     * background colors and the percentage is overlaid as centered text.
     *
     * When block characters are set (e.g. fillChar="█", halfChar="▓", emptyChar="░"),
     * sub-cell precision is used: the half-cell character appears at the boundary
     * when the fractional fill is >= 0.5, and the pct text overlay is suppressed.
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

        int w = spec.width;
        bool useBlockChars = (spec.fillChar != " " || spec.emptyChar != " ");

        if (!useBlockChars)
        {
            // Original space/bg-color bar with optional centered pct text overlay
            int filled = std::clamp((int)((float)current / total * w), 0, w);

            std::string bar(w, ' ');
            if (spec.showPct)
            {
                int         pct      = std::clamp((int)(100.0f * current / total), 0, 100);
                std::string pctStr   = std::to_string(pct) + "%";
                int         pctStart = (w - (int)pctStr.size()) / 2;
                for (int i = 0; i < (int)pctStr.size() && (pctStart + i) < w; ++i)
                    bar[pctStart + i] = pctStr[i];
            }

            if (filled > 0)
            {
                int pair = Colors::lookupColorPairByName(spec.fillFgColor, spec.fillBgColor);
                wattron(win, COLOR_PAIR(pair));
                mvwprintw(win, y, x, "%s", bar.substr(0, filled).c_str());
                wattroff(win, COLOR_PAIR(pair));
            }

            if (filled < w)
            {
                int pair = Colors::lookupColorPairByName(spec.emptyFgColor, spec.emptyBgColor);
                wattron(win, COLOR_PAIR(pair));
                mvwprintw(win, y, x + filled, "%s", bar.substr(filled).c_str());
                wattroff(win, COLOR_PAIR(pair));
            }
        }
        else
        {
            // Block-character bar with sub-cell precision via halfChar
            float fillF     = std::clamp((float)current / total * w, 0.0f, (float)w);
            int   fullCells = (int)fillF;
            float frac      = fillF - (float)fullCells;
            bool  hasHalf   = !spec.halfChar.empty() && frac >= 0.5f && fullCells < w;
            int   emptyStart = fullCells + (hasHalf ? 1 : 0);
            int   emptyCount = w - emptyStart;

            wmove(win, y, x);

            if (fullCells > 0)
            {
                std::string fill;
                for (int i = 0; i < fullCells; ++i) fill += spec.fillChar;
                int pair = Colors::lookupColorPairByName(spec.fillFgColor, spec.fillBgColor);
                wattron(win, COLOR_PAIR(pair));
                waddstr(win, fill.c_str());
                wattroff(win, COLOR_PAIR(pair));
            }

            if (hasHalf)
            {
                // Half cell: use fill foreground against empty background
                int pair = Colors::lookupColorPairByName(spec.fillFgColor, spec.emptyBgColor);
                wattron(win, COLOR_PAIR(pair));
                waddstr(win, spec.halfChar.c_str());
                wattroff(win, COLOR_PAIR(pair));
            }

            if (emptyCount > 0)
            {
                std::string empty;
                for (int i = 0; i < emptyCount; ++i) empty += spec.emptyChar;
                int pair = Colors::lookupColorPairByName(spec.emptyFgColor, spec.emptyBgColor);
                wattron(win, COLOR_PAIR(pair));
                waddstr(win, empty.c_str());
                wattroff(win, COLOR_PAIR(pair));
            }
        }
    }

} // namespace stevensTerminal
