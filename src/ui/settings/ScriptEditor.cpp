#include "ui/settings/ScriptEditor.hpp"
#include <algorithm>
#include <utility>
#include <vector>
#include "ui/settings/Controls.hpp"
#include "core/ResourceManager.hpp"
#include "ui/Theme.hpp"

namespace bh::settings_ui {
namespace {
constexpr float LineHeight = 30;
constexpr float FontSize = 22;
std::vector<std::size_t> lineStarts(const std::string &source) {
  std::vector<std::size_t> starts{0};
  for (std::size_t i = 0; i < source.size(); ++i) if (source[i] == '\n') starts.push_back(i + 1);
  return starts;
}
float width(std::string_view text) { return ResourceManager::measureText<Fonts_t::Buttons>(text, FontSize); }
bool key(int code) { return IsKeyPressed(code) || IsKeyPressedRepeat(code); }
}
void ScriptEditor::setText(std::string source) {
  m_text = std::move(source);
  m_cursor = 0;
  m_firstLine = 0;
  m_scrollX = 0;
  m_selectAll = false;
}
void ScriptEditor::keepVisible() {
  const auto starts = lineStarts(m_text);
  const int line = static_cast<int>(std::upper_bound(starts.begin(), starts.end(), m_cursor) - starts.begin()) - 1;
  const int visible = std::max(1, static_cast<int>((m_rect.height - 24) / LineHeight));
  if (line < m_firstLine) m_firstLine = line;
  if (line >= m_firstLine + visible) m_firstLine = line - visible + 1;
  const float cursorX = width(std::string_view(m_text).substr(starts[line], m_cursor - starts[line]));
  if (cursorX < m_scrollX) m_scrollX = cursorX;
  if (cursorX > m_scrollX + m_rect.width - 40) m_scrollX = cursorX - m_rect.width + 40;
}
void ScriptEditor::draw() const {
  panel(m_rect, theme::Background);
  DrawRectangleLinesEx(screenRect(m_rect), 2, m_focused ? theme::Highlight : theme::Border);
  const auto clip = screenRect({m_rect.x + 8, m_rect.y + 8, m_rect.width - 16, m_rect.height - 16});
  BeginScissorMode(static_cast<int>(clip.x), static_cast<int>(clip.y), static_cast<int>(clip.width), static_cast<int>(clip.height));
  const auto starts = lineStarts(m_text);
  for (int i = m_firstLine; i < static_cast<int>(starts.size()); ++i) {
    const float y = m_rect.y + 12 + (i - m_firstLine) * LineHeight;
    if (y >= m_rect.y + m_rect.height) break;
    const std::size_t end = i + 1 < static_cast<int>(starts.size()) ? starts[i + 1] - 1 : m_text.size();
    const auto line = std::string_view(m_text).substr(starts[i], end - starts[i]);
    if (m_selectAll) panel({m_rect.x + 10, y, m_rect.width - 20, LineHeight}, theme::Secondary);
    text(line, {m_rect.x + 14 - m_scrollX, y}, FontSize);
    if (m_focused && m_cursor >= starts[i] && m_cursor <= end && static_cast<int>(GetTime() * 2) % 2 == 0) {
      const float x = m_rect.x + 14 - m_scrollX + width(line.substr(0, m_cursor - starts[i]));
      panel({x, y, 2, FontSize + 2}, theme::Highlight);
    }
  }
  EndScissorMode();
}
bool ScriptEditor::input() {
  if (!IsWindowFocused()) { blur(); return false; }
  const auto mouse = mousePosition();
  const bool inside = CheckCollisionPointRec(mouse, m_rect);
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    m_focused = inside;
    m_selectAll = false;
    if (inside) {
      const auto starts = lineStarts(m_text);
      const int line = std::clamp(m_firstLine + static_cast<int>((mouse.y - m_rect.y - 12) / LineHeight), 0, static_cast<int>(starts.size()) - 1);
      const std::size_t end = line + 1 < static_cast<int>(starts.size()) ? starts[line + 1] - 1 : m_text.size();
      m_cursor = starts[line];
      const float target = mouse.x - m_rect.x - 14 + m_scrollX;
      while (m_cursor < end && width(std::string_view(m_text).substr(starts[line], m_cursor - starts[line] + 1)) < target) ++m_cursor;
    }
  }
  if (inside) {
    const int maximum = std::max(0, static_cast<int>(lineStarts(m_text).size()) - 1);
    m_firstLine = std::clamp(m_firstLine - static_cast<int>(GetMouseWheelMove() * 3), 0, maximum);
  }
  if (!m_focused) return false;
  const auto previous = m_text;
  const bool control = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  if (control && IsKeyPressed(KEY_A)) m_selectAll = true;
  if (control && IsKeyPressed(KEY_C) && m_selectAll) SetClipboardText(m_text.c_str());
  auto insert = [&](std::string value) {
    if (value.size() + (m_selectAll ? 0 : m_text.size()) > 65536) return;
    if (m_selectAll) { m_text.clear(); m_cursor = 0; m_selectAll = false; }
    m_text.insert(m_cursor, value);
    m_cursor += value.size();
  };
  if (control && IsKeyPressed(KEY_V)) {
    if (const char *clipboard = GetClipboardText()) {
      std::string pasted(clipboard);
      std::erase(pasted, '\r');
      insert(std::move(pasted));
    }
  }
  for (int ch = GetCharPressed(); ch; ch = GetCharPressed()) {
    if (!control && ch >= 32 && ch < 127) insert(std::string(1, static_cast<char>(ch)));
  }
  if (key(KEY_ENTER)) insert("\n");
  if (key(KEY_TAB)) insert("  ");
  if (key(KEY_BACKSPACE) || key(KEY_DELETE)) {
    if (m_selectAll) { m_text.clear(); m_cursor = 0; m_selectAll = false; }
    else if (key(KEY_BACKSPACE) && m_cursor > 0) m_text.erase(--m_cursor, 1);
    else if (key(KEY_DELETE) && m_cursor < m_text.size()) m_text.erase(m_cursor, 1);
  }
  if (key(KEY_LEFT) && m_cursor > 0) { --m_cursor; m_selectAll = false; }
  if (key(KEY_RIGHT) && m_cursor < m_text.size()) { ++m_cursor; m_selectAll = false; }
  if (key(KEY_UP) || key(KEY_DOWN) || key(KEY_HOME) || key(KEY_END)) {
    const auto starts = lineStarts(m_text);
    const int line = static_cast<int>(std::upper_bound(starts.begin(), starts.end(), m_cursor) - starts.begin()) - 1;
    const auto column = m_cursor - starts[line];
    const int next = std::clamp(line + (key(KEY_DOWN) ? 1 : key(KEY_UP) ? -1 : 0), 0, static_cast<int>(starts.size()) - 1);
    const auto end = next + 1 < static_cast<int>(starts.size()) ? starts[next + 1] - 1 : m_text.size();
    m_cursor = key(KEY_HOME) ? starts[next] : key(KEY_END) ? end : std::min(starts[next] + column, end);
    m_selectAll = false;
  }
  if (IsKeyPressed(KEY_ESCAPE)) blur();
  if (m_text != previous || GetKeyPressed() != 0) keepVisible();
  return previous != m_text;
}
} // namespace bh::settings_ui
