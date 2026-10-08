#include "states/BuildingState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"

namespace bh {

void BuildingState::draw() const noexcept {
  ResourceManager::drawImageTo<Textures::Building::Background>({0, 0, 1920, 1080});
  m_backButton.draw<WHITE, true, 40, TextAlign::Right>();
}

void BuildingState::update(float dt) noexcept { auto _ = dt; }

void BuildingState::events() noexcept {

  if (m_backButton.updateInput(GetMousePosition())) [[unlikely]] {
    m_stack.pop();
  }
}

void BuildingState::onEnter() noexcept {}

void BuildingState::onExit() noexcept {

  m_backButton.resetInteraction();
}

} // namespace bh
