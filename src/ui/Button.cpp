#include "ui/Button.hpp"

namespace bh {

bool Button::updateInput(Vector2 mousePosition, bool enabled) noexcept {
  if (!enabled || !IsWindowFocused()) {
    resetInteraction();
    return false;
  }

  const bool hovered = pressed(mousePosition);
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    m_armed = hovered;
  }
  const bool clicked =
      m_armed && hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
  const bool down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  if (!down) {
    m_armed = false;
  }
  m_visualState = !hovered            ? VisualState::Normal
                  : (m_armed && down) ? VisualState::Pressed
                                      : VisualState::Hover;
  return clicked;
}

void Button::resetInteraction() noexcept {
  m_armed = false;
  m_visualState = VisualState::Normal;
}

Rectangle Button::sourceRectangle() const noexcept {
  // Each group is normal, hover, pressed, matching
  // assets/textures/UI/Buttons.json.
  static constexpr Rectangle frames[][3] = {
      {{0, 0, 360, 100}, {0, 100, 360, 100}, {0, 200, 360, 100}},
      {{500, 0, 96, 96}, {648, 0, 96, 96}, {796, 0, 96, 96}},
      {{500, 100, 96, 96}, {648, 100, 96, 96}, {796, 100, 96, 96}},
      {{500, 200, 96, 96}, {648, 200, 96, 96}, {796, 200, 96, 96}},
      {{0, 300, 740, 100}, {0, 400, 740, 100}, {0, 500, 740, 100}},
      {{0, 600, 320, 96}, {336, 600, 320, 96}, {672, 600, 320, 96}},
      {{800, 300, 96, 96}, {800, 400, 96, 96}, {800, 500, 96, 96}},
      {{0, 700, 460, 100}, {0, 800, 460, 100}, {0, 900, 460, 100}},
  };
  for (const auto &variants : frames) {
    const auto &normal = variants[0];
    if (m_rect.x == normal.x && m_rect.y == normal.y &&
        m_rect.width == normal.width && m_rect.height == normal.height) {
      return variants[static_cast<std::uint8_t>(m_visualState)];
    }
  }
  return m_rect;
}

bool Button::pressed(Vector2 mousePosition) const noexcept {
  if (m_rect.width <= 0.f || m_rect.height <= 0.f) {
    return false;
  }
  const Vector2 pos = scaledSize(m_position);

  bool insideX = (pos.x <= mousePosition.x &&
                  mousePosition.x <
                      pos.x + scaledSize<float, ScreenAxis::X>(m_rect.width));

  bool insideY = (pos.y <= mousePosition.y &&
                  mousePosition.y <
                      pos.y + scaledSize<float, ScreenAxis::Y>(m_rect.height));

  return insideX && insideY;
}

} // namespace bh
