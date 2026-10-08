#include "states/MainMenuState.hpp"

#include "raylib.h"

#include "core/ResourceManager.hpp"
#include "core/StateStack.hpp"

#include "states/AchievementsState.hpp"
#include "states/BuildingState.hpp"
#include "states/GamemodeState.hpp"
#include "states/GuidesState.hpp"
#include "states/PauseMenuState.hpp"
#include "states/SettingsState.hpp"

#include "ui/Button.hpp"

namespace bh {

void MainMenuState::draw() const noexcept {
  ResourceManager::drawImageTo<Textures::MainMenu::Background>(
      {0, 0, 1920, 1080});
  ResourceManager::drawImage<Textures::MainMenu::Title>({0, 0, 600, 160},
                                                        {140, 250});
  m_playButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_quitButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_settingsButton.draw<WHITE, false>();
  m_guidesButton.draw<WHITE, false>();
  m_achievementsButton.draw<WHITE, false>();
}

void MainMenuState::update(float dt) noexcept { auto _ = dt; }

void MainMenuState::events() noexcept {

  const Vector2 MousePos = GetMousePosition();

  const bool playClicked = m_playButton.updateInput(MousePos);
  const bool quitClicked = m_quitButton.updateInput(MousePos);
  const bool settingsClicked = m_settingsButton.updateInput(MousePos);
  const bool guidesClicked = m_guidesButton.updateInput(MousePos);
  const bool achievementsClicked = m_achievementsButton.updateInput(MousePos);

  if (playClicked) [[unlikely]] {
    m_stack.push<GamemodeState>();

  } else if (quitClicked) [[unlikely]] {
    m_stack.quit = true;
    return;

  } else if (settingsClicked) [[unlikely]] {
    m_stack.push<SettingsState>();

  } else if (guidesClicked) [[unlikely]] {
    m_stack.push<GuidesState>();

  } else if (achievementsClicked) [[unlikely]] {
    m_stack.push<AchievementsState>();
  }
}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {
  m_playButton.resetInteraction();
  m_quitButton.resetInteraction();
  m_settingsButton.resetInteraction();
  m_guidesButton.resetInteraction();
  m_achievementsButton.resetInteraction();
}

} // namespace bh
