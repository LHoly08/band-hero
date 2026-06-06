#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

namespace bh {

class Button {
public:
  explicit Button(sf::Vector2f position, std::string &&texture) noexcept;
  void draw(sf::RenderTarget &target) const noexcept;
  bool pressed(const sf::Vector2f &mousePosition) const noexcept {
    auto size = m_texture.getSize();
    return (mousePosition.x <= (size.x + m_position.x) &&
            mousePosition.x >= m_position.x) &&
           (mousePosition.y <= (size.y + m_position.y) &&
            mousePosition.y >= m_position.y);
  }

private:
  sf::RenderTexture m_texture;
  sf::Vector2f m_position;
};

} // namespace bh
