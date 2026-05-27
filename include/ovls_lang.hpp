#pragma once
// ovlSysmodules (Ryazhenka) overlay-local i18n.
//
// All user-visible strings used by gui_main.cpp live as extern globals
// here so they can be swapped at startup based on system language. Loaded
// JSON files override the English defaults compiled into ovls_lang.cpp.
//
// JSON path on SD: /config/ryazhahand/ovlSysmodules/lang/<code>.json
//   where <code> is one of: en, ru.
//
// Falls back silently to compiled English defaults if a file is missing
// or malformed — the overlay never crashes due to a localisation issue.

#include <string>

namespace ovls {

    // Switch system-font glyph prefixes that upstream used in the
    // status string. NOT translatable — they're PUA codepoints that
    // render as the auto-start indicator icons on the console.
    //   ICON_NO_FLAG  =  U+E098 (clear)  + U+E031 (separator)
    //   ICON_FLAG     =  U+E0F4 (boot)   + U+E031 (separator)
    inline constexpr const char *ICON_NO_FLAG = "\xee\x82\x98\xee\x80\xb1";
    inline constexpr const char *ICON_FLAG    = "\xee\x83\xb4\xee\x80\xb1";

    extern std::string OVERLAY_TITLE;
    extern std::string ON_LABEL;
    extern std::string OFF_LABEL;
    extern std::string NO_SYSMODULES_FOUND;
    extern std::string SCAN_FAILED;
    extern std::string DYNAMIC_HEADER;
    extern std::string DYNAMIC_HINT;
    extern std::string STATIC_HEADER;
    extern std::string STATIC_HINT;

    // Call once at GUI construction. langCode is a two-letter ISO code
    // ("en", "ru"). If unrecognised or the file is missing, English
    // defaults remain in effect.
    void loadLanguage(const std::string &langCode);

} // namespace ovls
