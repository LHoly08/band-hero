#include "states/GamemodeState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/BuildingState.hpp"
#include "states/SongSelectState.hpp"
#include "ui/Button.hpp"

namespace bh {

void GamemodeState::draw() const noexcept {
  m_background.draw();
  m_localButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_lanButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_onlineButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_backButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void GamemodeState::update(float dt) noexcept { m_background.update(dt); }

void GamemodeState::events() noexcept {
  const Vector2 mousePos = GetMousePosition();
  
  const bool localClicked = m_localButton.updateInput(mousePos);
  const bool lanClicked = m_lanButton.updateInput(mousePos);
  const bool onlineClicked = m_onlineButton.updateInput(mousePos);
  const bool backClicked = m_backButton.updateInput(mousePos);

  if (localClicked) [[unlikely]] {
    m_stack.reset<SongSelectState>();

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
