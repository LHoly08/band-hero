#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>

#include "core/StateStack.hpp"

namespace bh {

class Game {
public:
  Game() noexcept;
  ~Game() = default;

  void run();

private:
  sf::RenderWindow m_window;
  sf::Clock m_clock;
  StateStack m_stack;

  void draw();
  void handleEvents();
  void update(float dt);
};

} // namespace bh
