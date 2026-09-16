#pragma once

#include <cstdint>
#include <type_traits>

#include "raylib.h"

namespace bh {

constexpr Vector2 OriginalWindowSize{.x = 1920, .y = 1080};

enum class ScreenAxis : std::uint8_t {
  X = 0,
  Y,
};

template <typename T>
concept Scalable = std::is_integral_v<T> || std::is_floating_point_v<T> ||
                   std::is_same_v<Vector2, T>;

template <Scalable T = Vector2, ScreenAxis Axis = ScreenAxis::X>
inline T scaledSize(T size) noexcept {

  if constexpr (std::is_same_v<Vector2, T>) {

    // Raylib drawing and mouse input both use logical screen coordinates.
    return Vector2{.x = (size.x * (GetScreenWidth() / OriginalWindowSize.x)),
                   .y = (size.y * (GetScreenHeight() / OriginalWindowSize.y))};

  } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {

    if constexpr (Axis == ScreenAxis::X) {

      return size * (GetScreenWidth() / OriginalWindowSize.x);

    } else if constexpr (Axis == ScreenAxis::Y) {

      return size * (GetScreenHeight() / OriginalWindowSize.y);
    }
  }
}

} // namespace bh
