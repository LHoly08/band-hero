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

// size 48 | align 8
class Button final {
public:
  inline Button(const Vector2 &position, const Rectangle &rect,
                std::string_view text = "")
      : m_text(text), m_rect(rect), m_position(position) {}

  inline ~Button() = default;

  inline void setPosition(const Vector2 &pos) noexcept { m_position = pos; }

  inline void changeTexture(const Rectangle &rect) noexcept { m_rect = rect; }

  inline void changeText(std::string_view text = "") noexcept {
    m_text = text;
  }

  template <Color Tint = WHITE, bool Text = true, int FontSize = 64,
            TextAlign TextAlignement = TextAlign::Left, Color TextColor = BLACK,
            Fonts_t FontType = Fonts_t::Default>
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

  ResourceManager::drawImage<Textures::UI::Buttons>(m_rect, m_position, Tint);

  if constexpr (Text) {
    if (m_text.empty() || m_rect.width <= 0.f || m_rect.height <= 0.f) {
      return;
    }

    const Vector2 size{m_rect.width, m_rect.height};
    const float padding = std::min({FontSize / 2.f, size.x / 4.f, size.y / 4.f});
    float fontSize = std::min(float(FontSize), size.y - 2.f * padding);
    float textWidth = ResourceManager::measureText<FontType>(m_text, fontSize);
    const float availableWidth = size.x - 2.f * padding;
    if (textWidth > availableWidth) {
      fontSize *= availableWidth / textWidth;
      textWidth = ResourceManager::measureText<FontType>(m_text, fontSize);
    }
    Vector2 textPos{m_position.x + padding,
                    m_position.y + ((size.y - fontSize) / 2.f)};

    if constexpr (TextAlignement == TextAlign::Center) {

      textPos.x = m_position.x + ((size.x - textWidth) / 2.f);

    } else if constexpr (TextAlignement == TextAlign::Right) {

      textPos.x = m_position.x + size.x - (padding + textWidth);
    }

    ResourceManager::drawText<FontType>(m_text, textPos, fontSize, TextColor);
  }
}

} // namespace bh
