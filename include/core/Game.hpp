#pragma once
#include <string>

#include "raylib.h"

#include "core/StateStack.hpp"
#include "config/DisplaySettings.hpp"
#include "core/ResourceManager.hpp"
#include "core/Scale.hpp"

#include "ui/Theme.hpp"

namespace bh {

class Game {
public:
  Game(const Vector2 &windowSize, const std::string_view &&windowName) noexcept;
  ~Game() noexcept;

  inline void run() {
    while (!WindowShouldClose()) {

      events();
      if (m_stack.quit) {
        break;
      }

      update(GetFrameTime());
      draw();

      m_stack.act();
    }
  }

private:
  inline void draw() const noexcept {
    BeginDrawing();

    ClearBackground(theme::Background);

    m_stack.draw();
    if (DisplaySettings::get().showFPS) {
      const Vector2 position = scaledSize(Vector2{1730, 20});
      const Vector2 size = scaledSize(Vector2{165, 48});
      DrawRectangleRounded({position.x, position.y, size.x, size.y}, .35f, 12,
                           Fade(theme::SecondaryBackground, .92f));
      ResourceManager::drawText<Fonts_t::Buttons>(
          std::to_string(GetFPS()) + " FPS", {1750, 32}, 24, theme::Highlight);
    }

    EndDrawing();
  }
  inline void update(float dt) noexcept { m_stack.update(dt); }
  inline void events() noexcept { m_stack.events(); }

  StateStack m_stack;
};

} // namespace bh
