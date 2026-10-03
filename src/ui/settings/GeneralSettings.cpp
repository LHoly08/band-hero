#include "ui/settings/GeneralSettings.hpp"
#include <algorithm>
#include <string>
#include "config/DisplaySettings.hpp"
#include "config/Settings.hpp"
#include "core/ResourceManager.hpp"
#include "ui/Theme.hpp"

namespace bh {
namespace {
constexpr Rectangle FPSPrevious{1220, 488, 60, 60};
constexpr Rectangle FPSNext{1660, 488, 60, 60};
constexpr Rectangle ResolutionPrevious{1220, 745, 60, 60};
constexpr Rectangle ResolutionNext{1660, 745, 60, 60};
constexpr Rectangle Fullscreen{1570, 840, 150, 50};
constexpr Rectangle Counter{1570, 588, 150, 50};

void selector(Rectangle previous, Rectangle next, const std::string &value) {
  using namespace settings_ui;
  choice(previous, "<");
  choice(next, ">");
  const float size = 32;
  const float width = ResourceManager::measureText<Fonts_t::Buttons>(value, size);
  text(value, {(previous.x + previous.width + next.x - width) / 2,
               previous.y + 14}, size, theme::Highlight);
}
}
void GeneralSettings::onEnter() {
  m_resolutions.clear();
  const int monitor = GetCurrentMonitor();
  const int maxWidth = GetMonitorWidth(monitor);
  const int maxHeight = GetMonitorHeight(monitor);
  for (Vector2 size : {Vector2{800, 600}, {1024, 768}, {1280, 720}, {1366, 768},
                      {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160}}) {
    if (size.x <= maxWidth && size.y <= maxHeight) m_resolutions.push_back(size);
  }
  const auto &display = DisplaySettings::get();
  m_resolutions.push_back({float(display.width), float(display.height)});
  if (maxWidth > 0 && maxHeight > 0) m_resolutions.push_back({float(maxWidth), float(maxHeight)});
  std::ranges::sort(m_resolutions, [](Vector2 a, Vector2 b) {
    return a.x == b.x ? a.y < b.y : a.x < b.x;
  });
  const auto duplicates = std::ranges::unique(m_resolutions, [](Vector2 a, Vector2 b) {
    return a.x == b.x && a.y == b.y;
  });
  m_resolutions.erase(duplicates.begin(), duplicates.end());

  m_refreshRate = GetMonitorRefreshRate(monitor);
  m_frameRates = {30, 60, 90, 120, 144, 165, 180, 240, 360};
  if (m_refreshRate > 0) m_frameRates.push_back(m_refreshRate);
  if (display.fps > 0) m_frameRates.push_back(display.fps);
  std::ranges::sort(m_frameRates);
  const auto repeated = std::ranges::unique(m_frameRates);
  m_frameRates.erase(repeated.begin(), repeated.end());
  m_frameRates.push_back(0);
}
void GeneralSettings::draw() const {
  using namespace settings_ui;
  const auto &display = DisplaySettings::get();
  text("General", {620, 150}, 48);
  text("Make the game feel right for your setup.", {620, 215}, 26, theme::MutedText);

  card({600, 275, 1180, 140});
  text("AUDIO", {630, 296}, 21, theme::Primary);
  text("Master volume", {640, 338}, 30);
  const int volume = Settings::getMasterVolume();
  m_volume.draw(volume, 100, theme::Highlight);
  text(volume == 0 ? "Muted" : std::to_string(volume) + "%", {1682, 338}, 24, theme::Highlight);

  card({600, 440, 1180, 220});
  text("PERFORMANCE", {630, 460}, 21, theme::Primary);
  text("Frame-rate limit", {640, 503}, 30);
  selector(FPSPrevious, FPSNext, display.fps == 0 ? "Unlimited" : std::to_string(display.fps) + " FPS");
  text(m_refreshRate > 0 ? "Monitor refresh rate: " + std::to_string(m_refreshRate) + " Hz" :
                          "Choose a frame-rate limit.",
       {640, 550}, 22, theme::MutedText);
  panel({630, 580, 1120, 1}, theme::Border);
  text("FPS counter", {640, 596}, 28);
  text("Show while playing", {1030, 600}, 22, theme::MutedText);
  toggle(Counter, display.showFPS);

  card({600, 685, 1180, 240});
  text("DISPLAY", {630, 705}, 21, theme::Primary);
  text("Resolution", {640, 760}, 30);
  selector(ResolutionPrevious, ResolutionNext,
           std::to_string(display.width) + " x " + std::to_string(display.height));
  panel({630, 825, 1120, 1}, theme::Border);
  text("Fullscreen", {640, 850}, 28);
  text(display.fullscreen ? "Use the whole screen" : "Play in a window",
       {1030, 855}, 22, theme::MutedText);
  toggle(Fullscreen, display.fullscreen);
}
bool GeneralSettings::events() {
  using namespace settings_ui;
  bool changed = false;
  int volume = Settings::getMasterVolume();
  if (m_volume.input(volume, 100)) { Settings::setMasterVolume(volume); changed = true; }
  auto &display = DisplaySettings::get();
  const int fpsStep = clicked(FPSNext) ? 1 : clicked(FPSPrevious) ? -1 : 0;
  if (fpsStep && !m_frameRates.empty()) {
    const auto current = std::ranges::find(m_frameRates, display.fps);
    const int count = static_cast<int>(m_frameRates.size());
    const int index = (static_cast<int>(current - m_frameRates.begin()) + fpsStep + count) % count;
    display.fps = m_frameRates[index];
    SetTargetFPS(display.fps);
    changed = true;
  }
  const int step = clicked(ResolutionNext) ? 1 : clicked(ResolutionPrevious) ? -1 : 0;
  if (step && !m_resolutions.empty()) {
    const auto current = std::ranges::find_if(m_resolutions, [&](Vector2 size) {
      return size.x == display.width && size.y == display.height;
    });
    const int count = static_cast<int>(m_resolutions.size());
    const int index = (static_cast<int>(current - m_resolutions.begin()) + step + count) % count;
    display.width = static_cast<int>(m_resolutions[index].x);
    display.height = static_cast<int>(m_resolutions[index].y);
    display.apply();
    return true;
  }
  if (clicked(Fullscreen)) {
    display.fullscreen = !display.fullscreen;
    display.apply();
    return true;
  }
  if (clicked(Counter)) { display.showFPS = !display.showFPS; changed = true; }
  return changed;
}
} // namespace bh
