#include <SFML/Window/Event.hpp>

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

StateStack::StateStack() {

  m_stack.reserve(3);
  push<MainMenuState>();
  act();
}

void StateStack::pop() noexcept { m_actions.push({Actions::POP, nullptr}); }

void StateStack::act() noexcept {

  while (!m_actions.empty()) {
    auto &front = m_actions.front();

    switch (front.type) {

    case Actions::PUSH: {

      if (!m_stack.empty()) {
        m_stack.back()->onExit();
      }
      m_stack.push_back(std::move(front.state));
      m_stack.back()->onEnter();

    } break;

    case Actions::POP: {

      if (!m_stack.empty()) {
        m_stack.back()->onExit();
        m_stack.pop_back();
        m_stack.empty() ? void() : m_stack.back()->onEnter();
      }

    } break;

    case Actions::REPLACE: {

      if (!m_stack.empty()) {
        m_stack.back()->onExit();
        m_stack.pop_back();
      }
      m_stack.push_back(std::move(front.state));
      m_stack.back()->onEnter();

    } break;
    }

    m_actions.pop();
  }
}

void StateStack::handleEvents(const sf::Event &event) noexcept {

  if (!m_stack.empty()) {
    m_stack.back()->handleEvents(event);
  }
}

void StateStack::update(float dt) noexcept {

  if (!m_stack.empty()) {
    m_stack.back()->update(dt);
  }
}

void StateStack::draw(sf::RenderTarget &target) const noexcept {

  for (const auto &state : m_stack) {
    state->draw(target);
  }
}

} // namespace bh
