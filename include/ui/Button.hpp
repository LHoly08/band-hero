#pragma once
#include <string>
#include <string_view>

#include "raylib.h"

namespace bh {

class Button final {
public:
  Button(std::string_view &&texturePath, Vector2 position,
         std::string_view text = "");
  inline ~Button() { UnloadTexture(m_texture); }

  void draw() const noexcept;
  bool pressed(Vector2 mousePosition) const noexcept;

private:
  Texture2D m_texture;
  std::string m_text;
  Vector2 m_position;
};

} // namespace bh
