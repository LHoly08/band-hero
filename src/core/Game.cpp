#include <filesystem>
#include <fstream>
#include <string_view>

#include "core/Game.hpp"

#include "raylib.h"

#include "core/ResourceManager.hpp"

#include "gameplay/Settings.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

Game::Game(const Vector2 &windowSize,
           const std::string_view &&windowName) noexcept {
  InitWindow(windowSize.x, windowSize.y, windowName.data());
  InitAudioDevice();

  SetExitKey(KeyboardKey::KEY_NULL);
  ToggleFullscreen();

  std::filesystem::path instrumentPaths("Instruments/");

  if (!std::filesystem::is_directory(instrumentPaths)) {
    std::filesystem::create_directory(instrumentPaths);
  }

  // TODO: make loading of config file
  if (std::ifstream file(Settings::startupFile); file.is_open()) {

  } else {
    Settings::defaultStartupSettings();
  }

  SetTargetFPS(60);

  m_stack.push<MainMenuState>();
  m_stack.act();
}

Game::~Game() noexcept {
  m_stack.clear();
  ResourceManager::unload();
  CloseAudioDevice();
  CloseWindow();
}

} // namespace bh
