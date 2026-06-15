#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <imgui.h>
#include <optional>

#include "states/MainMenuState.hpp"

#include "core/StateStack.hpp"

#include "instruments/Instrument.hpp"

#include "states/GameState.hpp"
#include "states/SettingsMenuState.hpp"

namespace bh {

MainMenuState::MainMenuState(StateStack &stack) noexcept : State(stack) {}

void MainMenuState::draw(sf::RenderTarget &target) const noexcept {}

void MainMenuState::handleEvents(const sf::Event &event) noexcept {

  if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {

    switch (keyPressed->scancode) {

    default:
      break;
    }
  } else if (const auto *mousePressed =
                 event.getIf<sf::Event::MouseButtonPressed>()) {
  }
}

void MainMenuState::update(const float dt) noexcept {

  if (ImGui::Button("Easy", ImVec2(100, 100))) {
    m_stack.replace<GameState<Difficulty::EASY>>("", nullptr);
  }

  ImGui::SameLine();

  if (ImGui::Button("Settings", ImVec2(100, 100))) {
    m_stack.replace<SettingsMenuState>();
  }
}

void MainMenuState::onEnter() noexcept {}

void MainMenuState::onExit() noexcept {}

} // namespace bh
