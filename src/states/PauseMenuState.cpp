#include "states/PauseMenuState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

#include "ui/Button.hpp"

namespace bh {

void PauseMenuState::draw() const noexcept {
  m_continueButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_mainMenuButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_quitButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void PauseMenuState::update(float dt) noexcept {
  m_delay += dt;
  if (m_clicked && m_delay >= 3) [[unlikely]] {

    m_stack.pop();
  }
}

void PauseMenuState::events() noexcept {

  const Vector2 MousePos = GetMousePosition();

  const bool continueClicked = m_continueButton.updateInput(MousePos);
  const bool quitClicked = m_quitButton.updateInput(MousePos);
  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);

  if (continueClicked || IsKeyPressed(KEY_ESCAPE)) [[unlikely]] {

    if (!m_clicked) [[likely]] {
      m_clicked = true;
      m_delay = 0;
    }

  } else if (quitClicked) [[unlikely]] {
    m_stack.quit = true;
    return;

  } else if (mainMenuClicked) [[unlikely]] {
    m_stack.reset<MainMenuState>();
  }
}

void PauseMenuState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();

  m_delay = 0;
  m_clicked = false;
}

void PauseMenuState::onExit() noexcept {
  ResourceManager::unloadTextures<Textures::UI>();

  m_continueButton.resetInteraction();
  m_quitButton.resetInteraction();
  m_mainMenuButton.resetInteraction();
}

} // namespace bh
