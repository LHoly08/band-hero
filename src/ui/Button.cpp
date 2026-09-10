#include "ui/Button.hpp"

namespace bh {

bool Button::pressed(Vector2 mousePosition) const noexcept {
  const Vector2 pos = scaledSize(m_position);

  bool insideX = (pos.x <= mousePosition.x &&
                  mousePosition.x <=
                      pos.x + scaledSize<float, ScreenAxis::X>(m_rect.width));

  bool insideY = (pos.y <= mousePosition.y &&
                  mousePosition.y <=
                      pos.y + scaledSize<float, ScreenAxis::Y>(m_rect.height));

  return insideX && insideY;
}

} // namespace bh
