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

    return Vector2{.x = (size.x * (GetRenderWidth() / OriginalWindowSize.x)),
                   .y = (size.y * (GetRenderHeight() / OriginalWindowSize.y))};

  } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {

    if constexpr (Axis == ScreenAxis::X) {

      return size * (GetRenderWidth() / OriginalWindowSize.x);

    } else if constexpr (Axis == ScreenAxis::Y) {

      return size * (GetRenderHeight() / OriginalWindowSize.y);
    }
  }
}

} // namespace bh
