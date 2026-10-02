#pragma once

#include "raylib.h"

namespace bh::theme {
// Neon Ultraviolet
inline constexpr Color Background{0x0C, 0x09, 0x14, 255};
inline constexpr Color SecondaryBackground{0x14, 0x10, 0x20, 255};
inline constexpr Color Panel{0x20, 0x18, 0x2F, 255};
inline constexpr Color Border{0x4C, 0x37, 0x6B, 255};
inline constexpr Color Primary{0xA9, 0x70, 0xFF, 255};
inline constexpr Color Secondary{0x74, 0x59, 0xE8, 255};
inline constexpr Color Highlight{0x36, 0xE2, 0xFF, 255};
inline constexpr Color Text{0xF5, 0xF0, 0xFF, 255};
inline constexpr Color MutedText{0xA7, 0x9A, 0xBF, 255};
inline constexpr Color Disabled{0x5B, 0x52, 0x6B, 255};
inline constexpr Color Error{0xFF, 0x52, 0x6F, 255};
inline constexpr Color Selected = Highlight;
} // namespace bh::theme
