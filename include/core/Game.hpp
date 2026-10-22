#pragma once
#include <string>

#include "raylib.h"

#include "config/DisplaySettings.hpp"

#include "core/ResourceManager.hpp"
#include "core/Scale.hpp"
#include "core/StateStack.hpp"

#include "ui/Theme.hpp"

namespace bh {

class Game {
public:
  Game(const ::Vector2 &windowSize, const std::string_view &&windowName) noexcept;
  ~Game() noexcept;

  inline void run() {
    while (!::WindowShouldClose()) {

      events();
      if (m_stack.quit) {
        break;
      }

      update(::GetFrameTime());
      draw();

      m_stack.act();
    }
  }

private:
  inline void draw() const noexcept {
    ::BeginDrawing();

    ::ClearBackground(bh::theme::Background);

    m_stack.draw();

    if (bh::DisplaySettings::get().showFPS) {
      const ::Vector2 position = bh::scaledSize(::Vector2{1730, 20});
      const ::Vector2 size = bh::scaledSize(::Vector2{165, 48});
      ::DrawRectangleRounded({position.x, position.y, size.x, size.y}, .35f, 12,
                           ::Fade(bh::theme::SecondaryBackground, .92f));
      bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(
          std::to_string(::GetFPS()) + " FPS", {1750, 32}, 24, bh::theme::Highlight);
    }

    ::EndDrawing();
  }
  inline void update(float dt) noexcept { m_stack.update(dt); }
  inline void events() noexcept { m_stack.events(); }

  bh::StateStack m_stack;
};

} // namespace bh
