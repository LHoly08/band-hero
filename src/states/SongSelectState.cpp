#include "states/SongSelectState.hpp"

#include "raylib.h"

#include "core/ResourceManager.hpp"
#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"
#include "states/PlayerSelectState.hpp"

#include "ui/Button.hpp"

namespace bh {

void SongSelectState::draw() const noexcept {

  m_mainMenuButton.draw<WHITE, true>();
  m_addSongButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void SongSelectState::update(float dt) noexcept { auto _ = dt; }

void SongSelectState::events() noexcept {

  const Vector2 MousePos{GetMousePosition()};
  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);
  const bool addSongClicked = m_addSongButton.updateInput(MousePos);

  if (mainMenuClicked) [[unlikely]] {
    m_stack.replace<MainMenuState>();

  } else if (addSongClicked) [[unlikely]] {
    m_stack.replace<PlayerSelectState>(s_SongsDir + "Bored");
  }
}

void SongSelectState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();
}

void SongSelectState::onExit() noexcept {
  m_mainMenuButton.resetInteraction();
  m_addSongButton.resetInteraction();
}

} // namespace bh
