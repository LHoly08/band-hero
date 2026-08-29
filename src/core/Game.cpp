#include <string_view>

#include "core/Game.hpp"

#include "raylib.h"

#include "states/MainMenuState.hpp"
#include <filesystem>

namespace bh {

Game::Game(const Vector2 &&windowSize,
           const std::string_view &&windowName) noexcept {
  InitWindow(windowSize.x, windowSize.y, windowName.data());
  SetExitKey(KeyboardKey::KEY_NULL);

  std::filesystem::path instrumentPaths("Instruments/");

  if (!std::filesystem::is_directory(instrumentPaths)) {
    std::filesystem::create_directory(instrumentPaths);
  }

  // TODO: make loading of config file
  SetTargetFPS(60);

  m_stack.push<MainMenuState>();
  m_stack.act();
}

Game::~Game() noexcept {
  m_stack.clear();
  CloseWindow();
}

} // namespace bh
