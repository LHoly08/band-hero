#include "states/PlayerSelectState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"
#include "states/GameState.hpp"

namespace bh {

void PlayerSelectState::draw() const noexcept {
  m_backNextButton.draw();
  m_increaseCountButton.draw();
  m_decreaseCountButton.draw();

  DrawText(&m_playerCount, 100, 200, 21, BLACK);
}

void PlayerSelectState::update(float dt) noexcept {}

void PlayerSelectState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_backNextButton.pressed(MousePos)) {

      m_choosingCount = !m_choosingCount;

    } else if (m_increaseCountButton.pressed(MousePos)) {

      const bool condition{(++m_playerCount) <= '4'};
      m_playerCount = (m_playerCount * condition) + ('1' * !condition);

    } else if (m_decreaseCountButton.pressed(MousePos)) {

      const bool condition{(--m_playerCount) >= '1'};
      m_playerCount = (m_playerCount * condition) + ('4' * !condition);

    } else if (m_startButton.pressed(MousePos)) {
      switch (m_playerCount) {
      case '1': {
        m_stack.push<GameState<1>>(
            std::array<std::unique_ptr<PlayerBase>, 1>{});
        break;
      }
      case '2': {
        m_stack.push<GameState<2>>(
            std::array<std::unique_ptr<PlayerBase>, 2>{});
        break;
      }
      case '3': {
        m_stack.push<GameState<3>>(
            std::array<std::unique_ptr<PlayerBase>, 3>{});
        break;
      }
      case '4': {
        m_stack.push<GameState<4>>(
            std::array<std::unique_ptr<PlayerBase>, 4>{});
        break;
      }
      }
    }
  }
}

void PlayerSelectState::onEnter() noexcept {}

void PlayerSelectState::onExit() noexcept {}

} // namespace bh
