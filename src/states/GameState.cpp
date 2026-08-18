#include <array>

#include "states/GameState.hpp"

#include "core/StateStack.hpp"

namespace bh {

void GameState::draw() const noexcept {}

void GameState::update(float dt) noexcept {

  if (std::uint32_t buffer{}; m_serial.readBytes(&buffer, sizeof(buffer), 1)) {

    switch (buffer & 0b11) {
    case 0: {
      buffer >>= 2;
      break;
    }
    case 1: {
      buffer >>= 2;
      break;
    }
    case 2: {
      buffer >>= 2;
      break;
    }
    case 3: {
      buffer >>= 2;
      break;
    }
    }
  }
}

void GameState::events() noexcept {}

void GameState::onEnter() noexcept {}

void GameState::onExit() noexcept {}

} // namespace bh
