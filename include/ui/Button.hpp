#pragma once
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>

#include "raylib.h"

#include "core/ResourceManager.hpp"
#include "core/Scale.hpp"

namespace bh {

enum class TextAlign : std::uint8_t {
  Left = 0,
  Center,
  Right,
};

// size 64 | align 8
class Button final {
public:
  inline Button(const Vector2 &position, const Rectangle &rect,
                std::string_view &&text = "")
      : m_text(text.data()), m_rect(rect), m_position(position) {}

  inline ~Button() = default;

  inline void changeTexture(const Rectangle &rect) noexcept { m_rect = rect; }

  inline void changeText(std::string_view &&text = "") noexcept {
    m_text = text.data();
  }

  template <Color Tint = WHITE, bool Text = false, int FontSize = 64,
            TextAlign TextAlignement = TextAlign::Left, Color TextColor = BLACK,
            Fonts_t FontType = Fonts_t::Buttons>
  void draw() const noexcept;
  bool pressed(Vector2 mousePosition) const noexcept;

private:
  std::string m_text;
  Rectangle m_rect;
  Vector2 m_position;
};

template <Color Tint, bool Text, int FontSize, TextAlign TextAlignement,
          Color TextColor, Fonts_t FontType>
void Button::draw() const noexcept {

  ResourceManager::drawImage<Textures_t::Buttons>(m_rect, m_position, Tint);

  if constexpr (Text) {

    const Vector2 size{m_rect.width, m_rect.height};
    const float padding = FontSize / 2.f;
    const float textHeight = FontSize;
    Vector2 textPos{m_position.x + padding,
                    m_position.y + ((size.y - textHeight) / 2.f)};

    if constexpr (TextAlignement == TextAlign::Center) {

      const float textWidth =
          ResourceManager::measureText<FontType>(m_text, FontSize);
      textPos.x = m_position.x + ((size.x - textWidth) / 2.f);

    } else if constexpr (TextAlignement == TextAlign::Right) {

      const float textWidth =
          ResourceManager::measureText<FontType>(m_text, FontSize);
      textPos.x = m_position.x + size.x - (padding + textWidth);
    }

    ResourceManager::drawText<FontType>(m_text, textPos, FontSize, TextColor);
  }
}

} // namespace bh
