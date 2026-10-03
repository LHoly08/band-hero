#include "ui/settings/Controls.hpp"
#include <algorithm>
#include <cmath>
#include "core/ResourceManager.hpp"
#include "core/Scale.hpp"
#include "ui/Theme.hpp"

namespace bh::settings_ui {
Rectangle screenRect(Rectangle rect) {
  const auto pos = scaledSize(Vector2{rect.x, rect.y});
  const auto size = scaledSize(Vector2{rect.width, rect.height});
  return {pos.x, pos.y, size.x, size.y};
}
Vector2 mousePosition() {
  const auto mouse = GetMousePosition();
  return {mouse.x * 1920.f / std::max(GetScreenWidth(), 1),
          mouse.y * 1080.f / std::max(GetScreenHeight(), 1)};
}
void panel(Rectangle rect, Color color) { DrawRectangleRec(screenRect(rect), color); }
void card(Rectangle rect) {
  ResourceManager::drawSettingsSkin<Textures::Settings::Panel>(
      {0, 0, 768, 384}, rect);
}
void toggle(Rectangle rect, bool enabled) {
  const bool hover = CheckCollisionPointRec(mousePosition(), rect);
  text(enabled ? "On" : "Off", {rect.x, rect.y + 12}, 26,
       enabled ? theme::Text : theme::MutedText);
  const Rectangle track{rect.x + rect.width - 86, rect.y + 4, 86, 42};
  DrawRectangleRounded(screenRect(track), 1.f, 16,
      enabled ? theme::Secondary : theme::Border);
  if (hover) DrawRectangleRoundedLinesEx(screenRect(track), 1.f, 16,
      scaledSize<float>(2.f), theme::Highlight);
  DrawRectangleRounded(screenRect({track.x + (enabled ? 48.f : 6.f),
      track.y + 5, 32, 32}), 1.f, 16, enabled ? theme::Highlight : theme::MutedText);
}
void text(std::string_view value, Vector2 pos, float size, Color color) {
  ResourceManager::drawText<Fonts_t::Buttons>(value, pos, size, color);
}
void fittedText(std::string_view value, Rectangle bounds, float size, Color color) {
  const float measured = ResourceManager::measureText<Fonts_t::Buttons>(value, size);
  if (measured > bounds.width && measured > 0) size *= bounds.width / measured;
  size = std::min(size, bounds.height);
  text(value, {bounds.x, bounds.y}, size, color);
}
void choice(Rectangle rect, std::string_view label, bool selected) {
  const bool hover = CheckCollisionPointRec(mousePosition(), rect);
  const float frame = selected ? 2.f : hover ? 1.f : 0.f;
  ResourceManager::drawSettingsSkin<Textures::Settings::Controls>(
      {0, frame * 128, 768, 128}, rect);
  float size = std::min(28.f, rect.height * .5f);
  const float measured = ResourceManager::measureText<Fonts_t::Buttons>(label, size);
  if (measured > rect.width - 32) size *= (rect.width - 32) / measured;
  const float offset = label.size() == 1 ?
      (rect.width - ResourceManager::measureText<Fonts_t::Buttons>(label, size)) / 2 : 16.f;
  text(label, {rect.x + offset, rect.y + (rect.height - size) / 2}, size);
}
bool clicked(Rectangle rect) {
  return IsWindowFocused() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
         CheckCollisionPointRec(mousePosition(), rect);
}
void Slider::draw(int value, int maximum, Color tint) const {
  const float width = m_rect.width * std::clamp(value / float(maximum), 0.f, 1.f);
  DrawRectangleRounded(screenRect(m_rect), 1.f, 12, theme::Border);
  if (width > 0) DrawRectangleRounded(screenRect({m_rect.x, m_rect.y, width, m_rect.height}), 1.f, 12, tint);
  const Rectangle knob{m_rect.x + width - 13, m_rect.y + m_rect.height / 2 - 13, 26, 26};
  const bool hover = CheckCollisionPointRec(mousePosition(), {knob.x - 8, knob.y - 8, 42, 42});
  if (hover || m_dragging) DrawRectangleRounded(screenRect({knob.x - 6, knob.y - 6, 38, 38}), 1.f, 16, Fade(tint, .2f));
  DrawRectangleRounded(screenRect(knob), 1.f, 16, theme::Text);
  DrawRectangleRoundedLinesEx(screenRect(knob), 1.f, 16, scaledSize<float>(3.f), tint);
}
bool Slider::input(int &value, int maximum) {
  if (!IsWindowFocused()) { reset(); return false; }
  if (clicked({m_rect.x - 16, m_rect.y - 20, m_rect.width + 32, m_rect.height + 40}))
    m_dragging = true;
  const int previous = value;
  if (m_dragging) {
    value = static_cast<int>(std::lround(std::clamp(
        (mousePosition().x - m_rect.x) / m_rect.width, 0.f, 1.f) * maximum));
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) reset();
  }
  return previous != value;
}
void TextField::draw() const {
  panel(m_rect, theme::Background);
  DrawRectangleLinesEx(screenRect(m_rect), 2, m_focused ? theme::Highlight : theme::Border);
  std::string shown = value;
  while (!shown.empty() && ResourceManager::measureText<Fonts_t::Buttons>(shown, 28) > m_rect.width - 40) {
    shown.erase(0, 1);
    while (!shown.empty() && (static_cast<unsigned char>(shown.front()) & 0xc0) == 0x80) shown.erase(0, 1);
  }
  text(shown + (m_focused && static_cast<int>(GetTime() * 2) % 2 == 0 ? "|" : ""),
       {m_rect.x + 14, m_rect.y + 14}, 28);
}
bool TextField::input() {
  if (!IsWindowFocused()) { blur(); return false; }
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    m_focused = CheckCollisionPointRec(mousePosition(), m_rect);
  if (!m_focused) return false;
  const auto previous = value;
  for (int ch = GetCharPressed(); ch != 0; ch = GetCharPressed()) {
    if (ch >= 32 && ch != 127) {
      int length{};
      const char *encoded = CodepointToUTF8(ch, &length);
      if (value.size() + length <= 64) value.append(encoded, length);
    }
  }
  if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !value.empty()) {
    while (!value.empty() && (static_cast<unsigned char>(value.back()) & 0xc0) == 0x80) value.pop_back();
    if (!value.empty())
    value.pop_back();
  }
  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) blur();
  return previous != value;
}
} // namespace bh::settings_ui
