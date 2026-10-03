#include "config/DisplaySettings.hpp"

#include <array>
#include <fstream>
#include "raylib.h"

namespace bh {
DisplaySettings &DisplaySettings::get() {
  static DisplaySettings settings = [] {
    DisplaySettings value;
    value.load();
    return value;
  }();
  return settings;
}

void DisplaySettings::load() {
  *this = DisplaySettings{};
  std::ifstream file("startup.bin", std::ios::binary);
  std::array<unsigned char, 10> bytes{};
  if (!file.read(reinterpret_cast<char *>(bytes.data()), 2)) return;
  const int storedFPS = bytes[0] | (bytes[1] << 8);
  if (storedFPS == 0 || (storedFPS >= 30 && storedFPS <= 1000)) fps = storedFPS;
  if (!file.read(reinterpret_cast<char *>(bytes.data() + 2), 8) ||
      bytes[2] != 'D' || bytes[3] != 1) return;
  const int w = bytes[4] | (bytes[5] << 8);
  const int h = bytes[6] | (bytes[7] << 8);
  if (w >= 640 && w <= 7680 && h >= 480 && h <= 4320) {
    width = w;
    height = h;
  }
  if (bytes[8] <= 1) fullscreen = bytes[8] != 0;
  if (bytes[9] <= 1) showFPS = bytes[9] != 0;
}

bool DisplaySettings::save() const {
  const std::array<unsigned char, 10> bytes{
      static_cast<unsigned char>(fps & 255), static_cast<unsigned char>(fps >> 8),
      'D', 1,
      static_cast<unsigned char>(width & 255), static_cast<unsigned char>(width >> 8),
      static_cast<unsigned char>(height & 255), static_cast<unsigned char>(height >> 8),
      static_cast<unsigned char>(fullscreen), static_cast<unsigned char>(showFPS)};
  std::ofstream file("startup.bin", std::ios::binary | std::ios::trunc);
  file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
  file.close();
  return !file.fail();
}

void DisplaySettings::apply() const {
  if (!IsWindowReady()) return;
  const bool resize = GetScreenWidth() != width || GetScreenHeight() != height;
  if (resize && IsWindowFullscreen()) ToggleFullscreen();
  if (resize) SetWindowSize(width, height);
  if (IsWindowFullscreen() != fullscreen) ToggleFullscreen();
  if (!fullscreen && resize) {
    const int monitor = GetCurrentMonitor();
    const Vector2 origin = GetMonitorPosition(monitor);
    SetWindowPosition(static_cast<int>(origin.x) + (GetMonitorWidth(monitor) - width) / 2,
                      static_cast<int>(origin.y) + (GetMonitorHeight(monitor) - height) / 2);
  }
  SetTargetFPS(fps);
}
} // namespace bh
