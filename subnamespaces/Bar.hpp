#pragma once
/**
 * @file Bar.hpp
 * @brief Progress bar and indicator utilities for stevensTerminal
 */

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>
#include "Colors.hpp"

#if defined(__linux__)
    #include <ncurses.h>
#elif defined(_WIN32) || defined(__MSDOS__)
    #include <curses.h>
#endif

namespace stevensTerminal
{
    /**
     * @brief One ordered segment of a bar's fill, in value-space out of some `total`.
     *
     * Sectors are drawn back-to-back in the order given (sector 0 starts at value 0, sector 1
     * starts where sector 0 ends, etc.), then whatever's left up to `total` renders as empty.
     * The classic two-state bar (fill/empty) is just the one-sector case.
     *
     * cellChars — this sector's own glyph ramp (see BarSpec::cellChars for the format). Leave
     *             empty to inherit BarSpec::cellChars, which covers the common case where every
     *             sector should share the same glyph precision and only the colors differ.
     */
    struct BarSector
    {
        float                     value;
        std::string               bgColor   = "green";
        std::string               fgColor   = "black";
        std::vector<std::string>  cellChars = {};
    };

    /**
     * @brief Spec for a printBar/createBarString call.
     *
     * Bars always render with block-character glyphs (cellChars) — there is no separate
     * plain-color mode. showPct/showFraction stamp a centered text overlay directly
     * over those glyphs: whichever columns the overlay text lands on get the overlay's
     * characters instead of a fill/empty glyph, but keep the same coloring the glyph would
     * have had at that column, so the overlay always reads correctly against the bar underneath
     * it. Overlay text is computed from the combined value of all sectors vs. `total`, so it's
     * only really meaningful for the single-sector fill/empty case (see the `int current, int
     * total` overload of createBarString) — pass showPct=false, showFraction=false for
     * multi-sector bars and compose any readout text yourself alongside the bar string.
     *
     * fillBgColor / fillFgColor   — ncurses color names for sector 0 when using the single-value
     *                               `createBarString(current, total, spec)` convenience overload
     * emptyBgColor / emptyFgColor — ncurses color names for the remaining (unfilled) portion
     * width                       — total character width of the bar
     * cellChars                   — the default glyph ramp for a single cell, from empty to full,
     *                               used by any sector that doesn't set its own BarSector::cellChars.
     *                               cellChars[0] is the empty glyph, cellChars[N] (N = size()-1) is
     *                               the full-cell glyph (also used to fill every fully-covered cell),
     *                               and each index in between is the boundary-cell glyph for a cell
     *                               filled to that many /N steps. cellChars.size()-1 IS the fill
     *                               precision (no separate count to keep in sync) — default is
     *                               {" ", leftHalfBlock, fullBlock} (precision 2: half/full block),
     *                               all confirmed CP437-safe/Consolas-safe. Pass the empty glyph plus
     *                               the 8 left-eighth-block glyphs from stevensTerminal::glyph for
     *                               smoother (but not Consolas-confirmed) eighth-cell precision. Must
     *                               have at least 2 entries.
     * showPct                     — overlay "45%" text centered on the bar
     * showFraction                — overlay "6/10" (current/total) text centered on the bar instead of
     *                               a percentage; takes priority over showPct
     */
    struct BarSpec
    {
        int                       width        = 40;
        std::string               fillBgColor  = "green";
        std::string               fillFgColor  = "black";
        std::string               emptyBgColor = "black";
        std::string               emptyFgColor = "white";
        std::vector<std::string>  cellChars    = { " ", std::string(glyph::leftHalfBlock), std::string(glyph::fullBlock) };
        bool                      showPct      = true;
        bool                      showFraction = false;
    };

    /**
     * @brief Builds a bar as a plain string of $[textColor=...,bgColor=...]-tokenized segments,
     *        suitable for embedding in any text that flows through the style-tokenizing print
     *        pipeline (e.g. cg_response.text), rather than drawn directly into a curses window.
     *
     * printBar() is a thin wrapper around this function. The single-value `(int current, int
     * total, spec)` overload below is a one-sector convenience call over this one.
     *
     * @param sectors Ordered fill segments, each in value-space out of `total` (see BarSector)
     * @param total   Maximum value the bar represents
     * @param spec    Visual options (empty-portion colors, width, default glyph ramp, overlay)
     * @return Tokenized bar string, or "" if total <= 0 or spec.cellChars has fewer than 2 entries
     */
    inline std::string createBarString(const std::vector<BarSector> & sectors, float total, const BarSpec & spec = {})
    {
        if (total <= 0 || spec.cellChars.size() < 2) { return ""; }

        int w = spec.width;

        // Resolve each sector's effective glyph ramp (own cellChars, or inherit spec's) up front.
        struct Resolved { float value; std::string bg, fg; const std::vector<std::string> * chars; int precision; };
        std::vector<Resolved> resolved;
        resolved.reserve(sectors.size());
        for (const auto & s : sectors)
        {
            const std::vector<std::string> * chars = (s.cellChars.size() >= 2) ? &s.cellChars : &spec.cellChars;
            resolved.push_back({ std::max(0.0f, s.value), s.bgColor, s.fgColor, chars, (int)chars->size() - 1 });
        }

        // Cumulative boundary column (float, in [0,w]) reached after each sector.
        std::vector<float> edges(resolved.size() + 1, 0.0f);
        float cum = 0.0f;
        for (size_t i = 0; i < resolved.size(); i++)
        {
            cum += resolved[i].value;
            edges[i + 1] = std::clamp(cum / total * (float)w, 0.0f, (float)w);
        }

        // Per-sector full-cell count and boundary-cell step. Sector-to-sector transitions are
        // hard edges snapped to the nearest whole cell -- each sector already reads as distinct
        // via its own color, so there's no need for a half-filled blend cell between them (and
        // rendering one would leave a stray sliver of that sector's own "empty" glyph/color
        // between two otherwise-adjacent colors). Only the final sector's trailing edge (where
        // the bar's fill actually gives way to real empty track) gets the soft partial-cell
        // treatment, same as the original single-sector algorithm. Either way, a genuinely
        // nonzero sector is nudged to claim at least one visible cell rather than fully
        // disappearing into the previous sector's boundary.
        std::vector<int> fullCells(resolved.size() + 1, 0);
        std::vector<int> boundaryStep(resolved.size() + 1, 0); // 0 = no partial boundary cell
        for (size_t i = 1; i <= resolved.size(); i++)
        {
            bool isLast = (i == resolved.size());
            if (!isLast)
            {
                int fc = std::clamp((int)std::lround(edges[i]), fullCells[i - 1], w);
                if (resolved[i - 1].value > 0.0f && fc == fullCells[i - 1])
                {
                    fc = std::min(w, fullCells[i - 1] + 1);
                }
                fullCells[i]    = fc;
                boundaryStep[i] = 0;
            }
            else
            {
                int   fc        = std::max((int)edges[i], fullCells[i - 1]);
                float frac      = edges[i] - (float)fc;
                int   precision = resolved[i - 1].precision;
                int   step      = (int)(frac * (float)precision);
                if (resolved[i - 1].value > 0.0f && fc == fullCells[i - 1] && step == 0)
                {
                    step = 1;
                }
                fullCells[i]    = fc;
                boundaryStep[i] = (step > 0 && fc < w) ? step : 0;
            }
        }

        // Which sector (if any) owns column `col`, and whether it's that sector's partial
        // boundary cell (later sectors win ties, so a shared boundary column always reflects
        // the furthest-along sector touching it).
        auto classify = [&](int col) -> std::pair<int, bool>
        {
            for (size_t i = 1; i <= resolved.size(); i++)
            {
                if (col < fullCells[i]) { return { (int)i - 1, false }; }
                if (col == fullCells[i] && boundaryStep[i] > 0) { return { (int)i - 1, true }; }
            }
            return { -1, false };
        };

        float totalFilled = cum;
        std::string overlay;
        if (spec.showFraction)      { overlay = std::to_string((int)std::lround(totalFilled)) + "/" + std::to_string((int)std::lround(total)); }
        else if (spec.showPct)      { overlay = std::to_string(std::clamp((int)(100.0f * totalFilled / total), 0, 100)) + "%"; }
        int overlayStart = std::max(0, (w - (int)overlay.size()) / 2);

        // A boundary cell's glyph is only PARTIALLY inked (e.g. a half-block) -- the uninked
        // portion is still literally empty track, so it must use the track's own emptyBgColor,
        // not the sector's bg (which exists to back a FULLY-inked cell and is otherwise never
        // seen through the glyph). Reusing the sector's bg there paints a stray patch of the
        // sector's backing color into what should read as track, producing a visible seam/hole
        // right at the fill boundary. fg is unaffected -- the inked portion is still the
        // sector's own ink color regardless of how much of the cell it covers.
        auto bgAt = [&](int col)
        {
            auto [idx, isBoundary] = classify(col);
            if (idx < 0 || isBoundary) { return spec.emptyBgColor; }
            return resolved[idx].bg;
        };
        auto fgAt = [&](int col) { int idx = classify(col).first; return idx >= 0 ? resolved[idx].fg : spec.emptyFgColor; };

        std::string result;
        int i = 0;
        while (i < w)
        {
            std::string bg = bgAt(i);
            std::string fg = fgAt(i);

            // Run-length: merge consecutive columns sharing this (fg,bg) pair into one segment.
            std::string segment;
            int j = i;
            while (j < w && bgAt(j) == bg && fgAt(j) == fg)
            {
                bool inOverlay = !overlay.empty() && j >= overlayStart && j < overlayStart + (int)overlay.size();
                if (inOverlay) { segment += overlay[j - overlayStart]; }
                else
                {
                    auto [idx, isBoundary] = classify(j);
                    if (idx < 0)          { segment += spec.cellChars.front(); }
                    else if (isBoundary)  { segment += (*resolved[idx].chars)[boundaryStep[idx + 1]]; }
                    else                  { segment += resolved[idx].chars->back(); }
                }
                ++j;
            }
            result += "{" + segment + "}$[textColor=" + fg + ",bgColor=" + bg + "]";
            i = j;
        }

        return result;
    }

    /**
     * @brief Single-value convenience overload: the classic two-state fill/empty bar, expressed
     *        as a one-sector call into the sector-based createBarString() above.
     *
     * @param current Current value
     * @param total   Maximum value
     * @param spec    Visual options (colors, width, characters)
     * @return Tokenized bar string, or "" if total <= 0 or spec.cellChars has fewer than 2 entries
     */
    inline std::string createBarString(int current, int total, const BarSpec & spec = {})
    {
        return createBarString(
            { BarSector{ (float)current, spec.fillBgColor, spec.fillFgColor, spec.cellChars } },
            (float)total, spec);
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

    /**
     * @brief Render a multi-sector bar (see createBarString(sectors, ...)) directly into a
     *        curses window.
     */
    inline void printBar(WINDOW*                        win,
                         int                             y,
                         int                             x,
                         const std::vector<BarSector> &  sectors,
                         float                           total,
                         const BarSpec &                 spec = {})
    {
        if (!win || total <= 0) return;

        std::string barString = createBarString(sectors, total, spec);
        PrintHelper::curses_wprint(win, y, x, barString, {}, {}, textStyling);
    }

} // namespace stevensTerminal