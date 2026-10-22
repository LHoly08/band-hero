#include "ui/settings/GeneralSettings.hpp"
#include <algorithm>
#include <string>
#include "config/DisplaySettings.hpp"
#include "config/Settings.hpp"
#include "core/ResourceManager.hpp"
#include "ui/Theme.hpp"

namespace bh {
namespace {
constexpr ::Rectangle FPSPrevious{1220, 473, 60, 60};
constexpr ::Rectangle FPSNext{1660, 473, 60, 60};
constexpr ::Rectangle ResolutionPrevious{1220, 650, 60, 60};
constexpr ::Rectangle ResolutionNext{1660, 650, 60, 60};
constexpr ::Rectangle Fullscreen{1570, 725, 150, 50};
constexpr ::Rectangle Counter{1570, 558, 150, 50};
constexpr ::Rectangle PortPrevious{1130, 850, 60, 50};
constexpr ::Rectangle PortNext{1510, 850, 60, 50};
constexpr ::Rectangle RefreshPorts{1590, 850, 150, 50};
constexpr ::Rectangle BaudPrevious{1130, 920, 60, 50};
constexpr ::Rectangle BaudNext{1510, 920, 60, 50};

void selector(::Rectangle previous, ::Rectangle next, const std::string &value) {

  bh::settings_ui::choice(previous, "<");
  bh::settings_ui::choice(next, ">");
  bh::settings_ui::fittedText(value, {previous.x + previous.width + 12, previous.y + 12,
                    next.x - previous.x - previous.width - 24, 36},
             32, bh::theme::Highlight);
}
}
void bh::GeneralSettings::onEnter() {
  refreshSerialPorts();
  m_resolutions.clear();
  const int monitor = ::GetCurrentMonitor();
  const int maxWidth = ::GetMonitorWidth(monitor);
  const int maxHeight = ::GetMonitorHeight(monitor);
  for (::Vector2 size : {::Vector2{800, 600}, {1024, 768}, {1280, 720}, {1366, 768},
                      {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160}}) {
    if (size.x <= maxWidth && size.y <= maxHeight) m_resolutions.push_back(size);
  }
  const auto &display = bh::DisplaySettings::get();
  m_resolutions.push_back({float(display.width), float(display.height)});
  if (maxWidth > 0 && maxHeight > 0) m_resolutions.push_back({float(maxWidth), float(maxHeight)});
  std::ranges::sort(m_resolutions, [](::Vector2 a, ::Vector2 b) {
    return a.x == b.x ? a.y < b.y : a.x < b.x;
  });
  const auto duplicates = std::ranges::unique(m_resolutions, [](::Vector2 a, ::Vector2 b) {
    return a.x == b.x && a.y == b.y;
  });
  m_resolutions.erase(duplicates.begin(), duplicates.end());

  m_refreshRate = ::GetMonitorRefreshRate(monitor);
  m_frameRates = {30, 60, 90, 120, 144, 165, 180, 240, 360};
  if (m_refreshRate > 0) m_frameRates.push_back(m_refreshRate);
  if (display.fps > 0) m_frameRates.push_back(display.fps);
  std::ranges::sort(m_frameRates);
  const auto repeated = std::ranges::unique(m_frameRates);
  m_frameRates.erase(repeated.begin(), repeated.end());
  m_frameRates.push_back(0);
}
void bh::GeneralSettings::refreshSerialPorts() {
  // Retain a configured but disconnected port, and put the explicit None option
  // first. Refresh updates choices without silently changing the saved device.
  m_serialPorts = bh::Settings::getAvailableSerialPorts();
  const auto &current = bh::Settings::getSerialPort();
  if (!current.empty() && !std::ranges::contains(m_serialPorts, current))
    m_serialPorts.push_back(current);
  m_serialPorts.insert(m_serialPorts.begin(), "");
}
void bh::GeneralSettings::draw() const {

  const auto &display = bh::DisplaySettings::get();
  bh::settings_ui::text("General", {620, 150}, 48);
  bh::settings_ui::text("Make the game feel right for your setup.", {620, 215}, 26, bh::theme::MutedText);

  bh::settings_ui::card({600, 275, 1180, 140});
  bh::settings_ui::text("AUDIO", {630, 296}, 21, bh::theme::Primary);
  bh::settings_ui::text("Master volume", {640, 338}, 30);
  const int volume = bh::Settings::getMasterVolume();
  m_volume.draw(volume, 100, bh::theme::Highlight);
  bh::settings_ui::text(volume == 0 ? "Muted" : std::to_string(volume) + "%", {1682, 338}, 24, bh::theme::Highlight);

  bh::settings_ui::card({600, 430, 1180, 180});
  bh::settings_ui::text("PERFORMANCE", {630, 445}, 21, bh::theme::Primary);
  bh::settings_ui::text("Frame-rate limit", {640, 478}, 30);
  selector(FPSPrevious, FPSNext, display.fps == 0 ? "Unlimited" : std::to_string(display.fps) + " FPS");
  bh::settings_ui::text(m_refreshRate > 0 ? "Monitor refresh rate: " + std::to_string(m_refreshRate) + " Hz" :
                          "Choose a frame-rate limit.",
       {640, 520}, 22, bh::theme::MutedText);
  bh::settings_ui::panel({630, 550, 1120, 1}, bh::theme::Border);
  bh::settings_ui::text("FPS counter", {640, 566}, 28);
  bh::settings_ui::text("Show while playing", {1030, 570}, 22, bh::theme::MutedText);
  bh::settings_ui::toggle(Counter, display.showFPS);

  bh::settings_ui::card({600, 625, 1180, 160});
  bh::settings_ui::text("DISPLAY", {630, 637}, 21, bh::theme::Primary);
  bh::settings_ui::text("Resolution", {640, 665}, 30);
  selector(ResolutionPrevious, ResolutionNext,
           std::to_string(display.width) + " x " + std::to_string(display.height));
  bh::settings_ui::panel({630, 715, 1120, 1}, bh::theme::Border);
  bh::settings_ui::text("Fullscreen", {640, 735}, 28);
  bh::settings_ui::text(display.fullscreen ? "Use the whole screen" : "Play in a window",
       {1030, 740}, 22, bh::theme::MutedText);
  bh::settings_ui::toggle(Fullscreen, display.fullscreen);

  bh::settings_ui::card({600, 800, 1180, 195});
  bh::settings_ui::text("SERIAL CONTROLLER", {630, 815}, 21, bh::theme::Primary);
  bh::settings_ui::text("Serial port", {640, 862}, 28);
  std::string port = bh::Settings::getSerialPort();
  if (port.starts_with("\\\\.\\")) port.erase(0, 4);
  selector(PortPrevious, PortNext, port.empty() ? "None" : port);
  bh::settings_ui::choice(RefreshPorts, "Refresh");
  bh::settings_ui::text("Baud rate", {640, 932}, 28);
  selector(BaudPrevious, BaudNext, std::to_string(bh::Settings::getSerialBaudRate()));
}
bool bh::GeneralSettings::events() {

  bool changed = false;
  int volume = bh::Settings::getMasterVolume();
  if (m_volume.input(volume, 100)) { bh::Settings::setMasterVolume(volume); changed = true; }
  auto &display = bh::DisplaySettings::get();
  const int fpsStep = bh::settings_ui::clicked(FPSNext) ? 1 : bh::settings_ui::clicked(FPSPrevious) ? -1 : 0;
  if (fpsStep && !m_frameRates.empty()) {
    const auto current = std::ranges::find(m_frameRates, display.fps);
    const int count = static_cast<int>(m_frameRates.size());
    const int index = (static_cast<int>(current - m_frameRates.begin()) + fpsStep + count) % count;
    display.fps = m_frameRates[index];
    ::SetTargetFPS(display.fps);
    changed = true;
  }
  const int step = bh::settings_ui::clicked(ResolutionNext) ? 1 : bh::settings_ui::clicked(ResolutionPrevious) ? -1 : 0;
  if (step && !m_resolutions.empty()) {
    const auto current = std::ranges::find_if(m_resolutions, [&](::Vector2 size) {
      return size.x == display.width && size.y == display.height;
    });
    const int count = static_cast<int>(m_resolutions.size());
    const int index = (static_cast<int>(current - m_resolutions.begin()) + step + count) % count;
    display.width = static_cast<int>(m_resolutions[index].x);
    display.height = static_cast<int>(m_resolutions[index].y);
    display.apply();
    return true;
  }
  if (bh::settings_ui::clicked(Fullscreen)) {
    display.fullscreen = !display.fullscreen;
    display.apply();
    return true;
  }
  if (bh::settings_ui::clicked(Counter)) { display.showFPS = !display.showFPS; changed = true; }
  if (bh::settings_ui::clicked(RefreshPorts)) refreshSerialPorts();
  const int portStep = bh::settings_ui::clicked(PortNext) ? 1 : bh::settings_ui::clicked(PortPrevious) ? -1 : 0;
  if (portStep && !m_serialPorts.empty()) {
    const auto current = std::ranges::find(m_serialPorts, bh::Settings::getSerialPort());
    const int count = static_cast<int>(m_serialPorts.size());
    const int index = (static_cast<int>(current - m_serialPorts.begin()) + portStep + count) % count;
    bh::Settings::setSerialPort(m_serialPorts[index]);
    changed = true;
  }
  const int baudStep = bh::settings_ui::clicked(BaudNext) ? 1 : bh::settings_ui::clicked(BaudPrevious) ? -1 : 0;
  if (baudStep) {
    const auto &rates = bh::Settings::supportedBaudRates;
    const auto current = std::ranges::find(rates, bh::Settings::getSerialBaudRate());
    const int count = static_cast<int>(rates.size());
    const int index = (static_cast<int>(current - rates.begin()) + baudStep + count) % count;
    changed = bh::Settings::setSerialBaudRate(rates[index]) || changed;
  }
  return changed;
}
} // namespace bh
