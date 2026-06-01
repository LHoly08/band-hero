#include "core/Game.hpp"

namespace bh {

Game::Game() noexcept : m_window(sf::VideoMode({1280, 720}), "Band-Hero") {

  m_window.setFramerateLimit(60);
}

void Game::run() {

  while (m_window.isOpen()) {
    sf::Time elapsed = m_clock.restart();

    handleEvents();
    update(elapsed.asSeconds());
    draw();
    m_stack.act();
  }
}

void Game::handleEvents() {

  while (const auto event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      m_window.close();
    } else {
      m_stack.handleEvents(*event);
    }
  }
}

void Game::update(float dt) { m_stack.update(dt); }

void Game::draw() { m_stack.draw(m_window); }

} // namespace bh
