#include <filesystem>
#include <fstream>
#include <string_view>

#include "core/Game.hpp"

#include "raylib.h"

#include "config/Settings.hpp"
#include "config/DisplaySettings.hpp"

#include "core/ResourceManager.hpp"

#include "gameplay/instruments/Instrument.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

Game::Game(const Vector2 &windowSize,
           const std::string_view &&windowName) noexcept {
  InitWindow(windowSize.x, windowSize.y, windowName.data());
  InitAudioDevice();
  SetMasterVolume(Settings::getMasterVolume() / 100.f);

  SetExitKey(KeyboardKey::KEY_NULL);
  DisplaySettings::get().apply();

  std::filesystem::path instrumentPaths("Instruments/");

  if (!std::filesystem::is_directory(instrumentPaths)) {
    std::filesystem::create_directory(instrumentPaths);
  }

  m_stack.push<MainMenuState>();
  m_stack.act();
}

Game::~Game() noexcept {
  // State exit hooks and texture unloading need live graphics/audio devices.
  // Tear down their owners before closing either device.
  m_stack.clear();
  ResourceManager::unload();
  CloseAudioDevice();
  CloseWindow();
}

} // namespace bh
