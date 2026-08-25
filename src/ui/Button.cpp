#include "ui/Button.hpp"

#include "raylib.h"

namespace bh {

Button::Button(std::string_view &&texturePath, Vector2 position,
               std::string_view text)
    : m_texture(LoadTexture(texturePath.data())), m_text(text.data()),
      m_position(position) {}

void Button::draw() const noexcept {
  DrawTexture(m_texture, m_position.x, m_position.y, WHITE);
}

bool Button::pressed(Vector2 mousePosition) const noexcept {
  bool insideX = (m_position.x <= mousePosition.x &&
                  mousePosition.x <= m_position.x + m_texture.width);

  bool insideY = (m_position.y <= mousePosition.y &&
                  mousePosition.y <= m_position.y + m_texture.height);

  return insideX && insideY;
}

} // namespace bh
