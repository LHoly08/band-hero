#pragma once

#include <string>
#include <string_view>
#include "raylib.h"

namespace bh::settings_ui {
Rectangle screenRect(Rectangle rect);
Vector2 mousePosition();
void panel(Rectangle rect, Color color);
void card(Rectangle rect);
void toggle(Rectangle rect, bool enabled);
void text(std::string_view value, Vector2 pos, float size = 30, Color color = {245, 240, 255, 255});
void fittedText(std::string_view value, Rectangle bounds, float size, Color color);
void choice(Rectangle rect, std::string_view label, bool selected = false);
bool clicked(Rectangle rect);

class Slider {
public:
  explicit Slider(Rectangle rect) : m_rect(rect) {}
  void draw(int value, int maximum, Color tint) const;
  bool input(int &value, int maximum);
  bool dragging() const { return m_dragging; }
  void reset() { m_dragging = false; }
private:
  Rectangle m_rect;
  bool m_dragging{};
};

class TextField {
public:
  explicit TextField(Rectangle rect) : m_rect(rect) {}
  std::string value;
  void draw() const;
  bool input();
  void blur() { m_focused = false; }
private:
  Rectangle m_rect;
  bool m_focused{};
};
} // namespace bh::settings_ui
