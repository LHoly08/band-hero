
#include <array>

#include "states/GuidesState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

#include "ui/Button.hpp"

namespace bh {

void GuidesState::draw() const noexcept {
  drawBackground();
  m_mainMenuButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_firstGuideButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_secondGuideButton.draw<WHITE, true, 40, TextAlign::Center>();
  m_thirdGuideButton.draw<WHITE, true, 40, TextAlign::Center>();
}

void GuidesState::update(float dt) noexcept { auto _ = dt; }

void GuidesState::events() noexcept {
  const Vector2 MousePos = GetMousePosition();

  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);
  const bool firstClicked = m_firstGuideButton.updateInput(MousePos);
  const bool secondClicked = m_secondGuideButton.updateInput(MousePos);
  const bool thirdClicked = m_thirdGuideButton.updateInput(MousePos);

  if (mainMenuClicked) [[unlikely]] {
    m_stack.replace<MainMenuState>();
    return;
  }

  if (firstClicked) [[unlikely]] {
    changeGuide(m_buttonGuides[0]);

  } else if (secondClicked) [[unlikely]] {
    changeGuide(m_buttonGuides[1]);

  } else if (thirdClicked) [[unlikely]] {
    changeGuide(m_buttonGuides[2]);
  }
}

void GuidesState::changeGuide(Guide newGuide) noexcept {
  constexpr std::array guides{Guide::Bass, Guide::Drums, Guide::Guitar,
                              Guide::Hub};
                              
  constexpr std::array<std::string_view, 4> names{
      "Bass Guide", "Drums Guide", "Guitar Guide", "Hub Guide"};
  const std::array buttons{&m_firstGuideButton, &m_secondGuideButton,
                           &m_thirdGuideButton};

  m_currentGuide = newGuide;
  std::size_t buttonIndex = 0;
  for (const Guide guide : guides) {
    if (guide == m_currentGuide) {
      continue;
    }

    m_buttonGuides[buttonIndex] = guide;
    buttons[buttonIndex]->changeText(names[static_cast<std::size_t>(guide)]);
    buttons[buttonIndex]->resetInteraction();
    ++buttonIndex;
  }
}

void GuidesState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();
}

void GuidesState::onExit() noexcept {
  ResourceManager::unloadTextures<Textures::UI>();

  m_mainMenuButton.resetInteraction();
  m_firstGuideButton.resetInteraction();
  m_secondGuideButton.resetInteraction();
  m_thirdGuideButton.resetInteraction();
}

} // namespace bh
