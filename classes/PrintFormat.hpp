#pragma once
/**
 * A typed, defaulted alternative to the string-keyed format maps that printVector_str(),
 * curses_mvw_printVector(), curses_wprint(), and curses_wwrap_withTokens() accept. Every field
 * here has a default matching what that string-keyed map's parsing code already falls back to
 * when its corresponding key is absent, so PrintFormat{} behaves identically to passing {}.
 *
 * Use designated initializers to set only what you care about:
 *   PrintFormat{.columns = 2, .rows = 4, .defaultColumnWidth = "auto"}
 *
 * A forgotten field simply keeps its default - unlike the map, there's no key to forget, and no
 * way to typo one ("horizontal seperator" silently doing nothing was a real, hard-to-diagnose bug
 * this struct exists to make impossible).
 *
 * TODO (deferred, not started): several fields below are still closed-set strings
 * (sequence, listType, defaultColumnWidth, textAlign) - a typo in one of these silently
 * falls through to a default the same way the old map keys did, just one layer in. Once
 * this struct is used broadly enough to be worth a dedicated pass, convert each to an enum
 * (e.g. `enum class FillSequence { ColumnMajor, RowMajor }`) and update toFormatMap() to
 * stringify it at the boundary, rather than storing the string form directly. This is part
 * of the larger "options structs instead of string-keyed maps" modernization for
 * stevensTerminal's API (formatTableAsString, curses_wprint, curses_wwrap_withTokens, etc.
 * still take raw string-keyed maps) - PrintFormat is a first step, not the finished state.
 */

#include <string>
#include <unordered_map>

namespace stevensTerminal {

struct PrintFormat
{
    /*** Grid layout - consumed by printVector_str() ***/
    int columns = -1;                          // number of columns; -1 = unset (single column)
    int rows = -1;                              // number of rows; -1 = unset (as many as needed)
    bool allowOverflow = false;                 // print elements past columns*rows instead of dropping them
    std::string sequence = "column-major";      // "column-major" or "row-major" grid-fill order
    std::string listType = "";                  // "" or "numbered" (prefixes each element with its index)
    std::string prependString = "";             // text prepended to every element
    std::string appendString = "";              // text appended to every element
    // "none" (no padding), "auto" (pad to each column's own longest cell), or a plain integer
    // string (pad every column - without its own columnWidths entry - to that exact width).
    std::string defaultColumnWidth = "none";
    std::string horizontalSeparator = "\t";     // inserted BETWEEN columns only, never trailing

    /*** Rendering - consumed by curses_wprint() / curses_wwrap_withTokens() ***/
    bool wrap = false;                          // wrap text to the window width instead of truncating
    bool avoidBorders = false;                  // keep printed content clear of a 1-cell border
    bool retainXMoveOnNewline = false;          // resume at the original xMove after a wrap/newline instead of column 0
    std::string textAlign = "left";             // "left", "center", or "right" (right: reserved, unimplemented)

    /**
     * @brief Converts to the legacy string-keyed format map, so struct-based callers can share
     *        the same (well-tested) implementation as map-based ones instead of duplicating it.
     *        columns/rows are omitted when unset (-1) rather than emitted as "-1", matching how
     *        the map-parsing code already treats an absent key.
     */
    std::unordered_map<std::string,std::string> toFormatMap() const
    {
        std::unordered_map<std::string,std::string> format;
        if(columns >= 0) { format["columns"] = std::to_string(columns); }
        if(rows >= 0)    { format["rows"]    = std::to_string(rows); }
        format["allowOverflow"] = allowOverflow ? "true" : "false";
        format["sequence"] = sequence;
        if(!listType.empty())      { format["listType"] = listType; }
        if(!prependString.empty()) { format["prependString"] = prependString; }
        if(!appendString.empty())  { format["appendString"] = appendString; }
        format["defaultColumnWidth"] = defaultColumnWidth;
        format["horizontal separator"] = horizontalSeparator;
        format["wrap"] = wrap ? "true" : "false";
        format["avoid borders"] = avoidBorders ? "true" : "false";
        format["retain xmove on newline"] = retainXMoveOnNewline ? "true" : "false";
        format["textAlign"] = textAlign;
        return format;
    }
};

} // namespace stevensTerminal