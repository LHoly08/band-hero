#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

namespace bh {

class Button {
public:
  explicit Button(sf::Vector2f position, sf::Vector2u size,
                  std::string &&textureFile, std::string &&buttonText,
                  std::string &&fontFile) noexcept;
  void draw(sf::RenderTarget &target) const noexcept;
  bool pressed(const sf::Vector2i &mousePosition) const noexcept;

private:
  sf::RenderTexture m_texture;
  const sf::Vector2f m_position;
};

} // namespace bh
