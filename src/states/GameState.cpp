#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "states/GameState.hpp"

namespace bh {

/* Gamemode Easy */

template <>
void GameState<Difficulty::EASY>::draw(
    sf::RenderTarget &target) const noexcept {

  target.clear(sf::Color::Red);
}

/* Gamemode Hard */

template <>
void GameState<Difficulty::HARD>::draw(
    sf::RenderTarget &target) const noexcept {

  target.clear(sf::Color::Green);
}

} // namespace bh
