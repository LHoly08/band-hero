#pragma once
#include <string>
#include <string_view>

#include "raylib.h"

namespace bh {

// size 64 | align 8
class Button final {
public:
  Button(std::string_view &&texturePath, Vector2 position,
         std::string_view &&text = "");
  inline ~Button() { UnloadTexture(m_texture); }

  inline void changeTexture(std::string_view &&texturePath) noexcept;
  inline void changeText(std::string_view &&text = "") noexcept;
  void draw() const noexcept;
  bool pressed(Vector2 mousePosition) const noexcept;

private:
  std::string m_text;
  Texture2D m_texture;
  Vector2 m_position;
};

} // namespace bh
