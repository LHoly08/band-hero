#include "states/PauseMenuState.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "raylib.h"

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

#include "ui/Button.hpp"
#include "ui/Theme.hpp"

namespace bh {
namespace {
constexpr float ResumeDelay = 3.f;
}

void bh::PauseMenuState::draw() const noexcept {
  bh::ResourceManager::drawSettingsSkin<bh::Textures::UI::Panel>(
    {0, 0, 768, 384}, {570, 410, 780, 260}
  );

  if (m_clicked) {
    const auto centeredText = [](std::string_view text, float y, float size,
                                 ::Color color) {
      const float width = bh::ResourceManager::measureText<bh::Fonts_t::Buttons>(text, size);
      bh::ResourceManager::drawText<bh::Fonts_t::Buttons>(text, {960 - width / 2, y},
                                                 size, color);
    };
    const float remaining = std::max(0.f, ResumeDelay - m_delay);
    const int seconds = std::clamp(static_cast<int>(std::ceil(remaining)), 1, 3);
    centeredText("RESUMING", 435, 26, bh::theme::Text);
    centeredText(std::to_string(seconds), 475, 100, bh::theme::Highlight);
    const auto position = bh::scaledSize(::Vector2{710, 610});
    const auto size = bh::scaledSize(::Vector2{500, 10});
    ::DrawRectangleRounded({position.x, position.y, size.x, size.y}, 1.f, 12,
                         bh::theme::Border);
    if (remaining > 0.f) {
      ::DrawRectangleRounded({position.x, position.y,
                            size.x * remaining / ResumeDelay, size.y},
                           1.f, 12, bh::theme::Highlight);
    }
    centeredText("Press Esc to stay paused", 637, 18, bh::theme::MutedText);
    return;
  }

  m_continueButton.draw<WHITE, true, 40, bh::TextAlign::Center>();
  m_mainMenuButton.draw<WHITE, true, 40, bh::TextAlign::Center>();
  m_quitButton.draw<WHITE, true, 40, bh::TextAlign::Center>();
}

void bh::PauseMenuState::update(float dt) noexcept {
  if (!m_clicked) return;
  m_delay += dt;
  if (m_delay >= ResumeDelay) [[unlikely]] {

    m_stack.pop();
  }
}

void bh::PauseMenuState::events() noexcept {

  if (m_clicked) {
    // Countdown replaces the menu, so hidden buttons must not accept clicks.
    if (::IsKeyPressed(::KEY_ESCAPE)) {
      m_clicked = false;
      m_delay = 0;
    }
    return;
  }

  const ::Vector2 MousePos = ::GetMousePosition();

  const bool continueClicked = m_continueButton.updateInput(MousePos);
  const bool quitClicked = m_quitButton.updateInput(MousePos);
  const bool mainMenuClicked = m_mainMenuButton.updateInput(MousePos);

  if (continueClicked || ::IsKeyPressed(::KEY_ESCAPE)) [[unlikely]] {

    if (!m_clicked) [[likely]] {
      m_clicked = true;
      m_delay = 0;
      m_continueButton.resetInteraction();
      m_quitButton.resetInteraction();
      m_mainMenuButton.resetInteraction();
    }

  } else if (quitClicked) [[unlikely]] {
    m_stack.quit = true;
    return;

  } else if (mainMenuClicked) [[unlikely]] {
    m_stack.reset<bh::MainMenuState>();
  }
}

void bh::PauseMenuState::onEnter() noexcept {

  m_delay = 0;
  m_clicked = false;
}

void bh::PauseMenuState::onExit() noexcept {

  m_continueButton.resetInteraction();
  m_quitButton.resetInteraction();
  m_mainMenuButton.resetInteraction();
}

} // namespace bh
