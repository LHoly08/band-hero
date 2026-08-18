#include <any>

#include "core/Game.hpp"

#include "raylib.h"

namespace bh {

Game::Game(const Vector2 &&windowSize,
           const std::string &&windowName) noexcept {
  InitWindow(windowSize.x, windowSize.y, windowName.c_str());
  SetExitKey(KeyboardKey::KEY_NULL);

  // TODO: make loading of config file
  SetTargetFPS(60);
}

Game::~Game() noexcept { CloseWindow(); }

} // namespace bh
