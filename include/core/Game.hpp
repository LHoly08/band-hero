#pragma once
#include <SFML/Window/WindowEnums.hpp>

#include <string>

#include "raylib.h"

#include "core/StateStack.hpp"

namespace bh {

class Game {
public:
  Game(const Vector2 &&windowSize, const std::string &&windowName) noexcept;
  ~Game() noexcept;

  inline void run() {
    while (!WindowShouldClose()) {

      events();
      update(GetFrameTime());
      draw();
      m_stack.act();
    }
  }

private:
  inline void draw() const noexcept {
    BeginDrawing();

    m_stack.draw();

    EndDrawing();
  }
  inline void update(float dt) noexcept { m_stack.update(dt); }
  inline void events() noexcept { m_stack.events(); }

  StateStack m_stack;
};

} // namespace bh
