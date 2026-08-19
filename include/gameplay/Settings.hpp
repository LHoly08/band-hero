#pragma once
#include "raylib.h"
#include <array>
#include <cstdint>

namespace bh {

class Settings {
public:
  inline static Settings &get() {
    static Settings s_instance{};
    return s_instance;
  }

  inline static Color getNoteTint(std::uint8_t index) noexcept {
    return get().iGetNoteTint(index);
  }

  inline static void loadSettings() noexcept { return get().iLoadSettings(); }
  inline static void saveSettings() noexcept { return get().iSaveSettings(); }

  ~Settings() = default;

  Settings operator=(const Settings &) = delete;
  Settings(const Settings &) = delete;
  Settings operator=(Settings &&) = delete;
  Settings(Settings &&) = delete;

private:
  Settings();

  void iLoadSettings() noexcept;
  void iSaveSettings() noexcept;

  inline Color iGetNoteTint(std::uint8_t index) const noexcept {
    try {
      return guitarBassColors.at(index);
    } catch (...) {
      return guitarBassColors[0];
    }
  }
  std::array<Color, 6> guitarBassColors{RED,   ORANGE, YELLOW,
                                        GREEN, BLUE,   PURPLE};
};

} // namespace bh