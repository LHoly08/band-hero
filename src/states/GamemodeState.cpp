#include "states/GamemodeState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/BuildingState.hpp"
#include "states/SongSelectState.hpp"

namespace bh {

void GamemodeState::draw() const noexcept {
  m_localButton.draw<WHITE, true>();
  m_lanButton.draw<WHITE, true>();
  m_onlineButton.draw<WHITE, true>();
  m_backButton.draw<WHITE, true>();
}

void GamemodeState::update(float dt) noexcept { auto _ = dt; }

void GamemodeState::events() noexcept {
  const Vector2 mousePos = GetMousePosition();
  const bool localClicked = m_localButton.updateInput(mousePos);
  const bool lanClicked = m_lanButton.updateInput(mousePos);
  const bool onlineClicked = m_onlineButton.updateInput(mousePos);
  const bool backClicked = m_backButton.updateInput(mousePos);

  if (localClicked) [[unlikely]] {
    m_stack.pop();
    m_stack.replace<SongSelectState>();

  } else if (lanClicked) [[unlikely]] {
    m_stack.push<BuildingState>();

  } else if (onlineClicked) [[unlikely]] {
    m_stack.push<BuildingState>();

  } else if (backClicked) [[unlikely]] {
    m_stack.pop();
  }
}

void GamemodeState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();
}

void GamemodeState::onExit() noexcept {
  ResourceManager::unloadTextures<Textures::UI>();

  m_localButton.resetInteraction();
  m_lanButton.resetInteraction();
  m_onlineButton.resetInteraction();
  m_backButton.resetInteraction();
}

} // namespace bh
