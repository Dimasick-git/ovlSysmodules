#pragma once

#include <string>

namespace ovls {

// Значки шрифта Nintendo Switch: не переводятся и не учитываются как текст.
inline constexpr const char *ICON_NO_FLAG = "\xee\x82\x98\xee\x80\xb1";
inline constexpr const char *ICON_FLAG    = "\xee\x83\xb4\xee\x80\xb1";

extern const std::string OVERLAY_TITLE;
extern const std::string ON_LABEL;
extern const std::string OFF_LABEL;
extern const std::string NO_SYSMODULES_FOUND;
extern const std::string SCAN_FAILED;
extern const std::string DYNAMIC_HEADER;
extern const std::string DYNAMIC_HINT;
extern const std::string STATIC_HEADER;
extern const std::string STATIC_HINT;
extern const std::string SHUTDOWN_IPC_FAILED;
extern const std::string RAM_LABEL;
extern const std::string FREE_LABEL;

} // namespace ovls
