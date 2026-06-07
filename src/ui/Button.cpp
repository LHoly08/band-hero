#include "ui/Button.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace bh {

Button::Button(sf::Vector2f position, sf::Vector2u size,
               std::string &&textureFile, std::string &&buttonText,
               std::string &&fontFile) noexcept
    : m_texture(size), m_position(position) {

  sf::Texture texture(textureFile);

  sf::Sprite sprite(texture);
  sf::Vector2f scale = {(size.x / static_cast<float>(texture.getSize().x)),
                        (size.y / static_cast<float>(texture.getSize().y))};
  sprite.setScale(scale);

  sf::Font font(fontFile);
  sf::Text text(font, buttonText);
  text.setScale(scale);

  sf::FloatRect textRect = text.getLocalBounds();

  text.setOrigin({textRect.position.x + textRect.size.x / 2.0f,
                  textRect.position.y + textRect.size.y / 2.0f});

  text.setPosition({size.x / 2.0f, size.y / 2.0f});

  m_texture.clear();
  m_texture.draw(sprite);
  m_texture.draw(text);
  m_texture.display();
}

void Button::draw(sf::RenderTarget &target) const noexcept {
  sf::Sprite sprite(m_texture.getTexture());
  sprite.setPosition(m_position);
  target.draw(sprite);
}

bool Button::pressed(const sf::Vector2i &mousePosition) const noexcept {
  auto size = m_texture.getSize();
  return (mousePosition.x <= (size.x + m_position.x) &&
          mousePosition.x >= m_position.x) &&
         (mousePosition.y <= (size.y + m_position.y) &&
          mousePosition.y >= m_position.y);
}

} // namespace bh