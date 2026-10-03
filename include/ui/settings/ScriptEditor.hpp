#pragma once
#include <string>
#include "raylib.h"

namespace bh::settings_ui {
class ScriptEditor {
public:
  explicit ScriptEditor(Rectangle rect) : m_rect(rect) {}
  void setText(std::string text);
  const std::string &value() const { return m_text; }
  void draw() const;
  bool input();
  void blur() { m_focused = false; m_selectAll = false; }
private:
  void keepVisible();
  Rectangle m_rect;
  std::string m_text;
  std::size_t m_cursor{};
  int m_firstLine{};
  float m_scrollX{};
  bool m_focused{};
  bool m_selectAll{};
};
} // namespace bh::settings_ui
