#include "states/BuildingState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "ui/Button.hpp"

namespace bh {

void bh::BuildingState::draw() const noexcept {
  bh::ResourceManager::drawImageTo<bh::Textures::Building::Background>({0, 0, 1920, 1080});
  m_backButton.draw<WHITE, true, 40, bh::TextAlign::Right>();
}

void bh::BuildingState::update(float dt) noexcept { auto _ = dt; }

void bh::BuildingState::events() noexcept {

  if (m_backButton.updateInput(::GetMousePosition())) [[unlikely]] {
    m_stack.pop();
  }
}

void bh::BuildingState::onEnter() noexcept {}

void bh::BuildingState::onExit() noexcept {

  m_backButton.resetInteraction();
}

} // namespace bh
