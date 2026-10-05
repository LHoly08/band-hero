#include "ui/settings/GameplaySettings.hpp"
#include <string>
#include "config/Settings.hpp"
#include "core/ResourceManager.hpp"
#include "ui/Theme.hpp"

namespace bh {
void GameplaySettings::draw() const {
  using namespace settings_ui;
  text("Gameplay", {620, 150}, 48);
  text("Choose a note color, then drag its red, green and blue sliders.", {620, 215}, 26, theme::MutedText);
  card({600, 270, 1180, 210});
  card({600, 495, 1180, 400});
  for (int i = 0; i < 6; ++i) {
    choice({650 + i * 170.f, 295, 140, 160}, "", i == m_note);
    ResourceManager::drawImage<Textures::Gameplay::Notes>(
        {150.f * (i % 4), 0, 150, 150}, {645 + i * 170.f, 280}, Settings::getNoteTint(i));
    text(std::to_string(i + 1), {710 + i * 170.f, 425}, 24);
  }
  const Color color = Settings::getNoteTint(m_note);
  const int values[]{color.r, color.g, color.b};
  const char *labels[]{"Red", "Green", "Blue"};
  const Color tints[]{RED, GREEN, BLUE};
  text("NOTE " + std::to_string(m_note + 1) + " COLOR", {630, 515}, 22, theme::Primary);
  for (int i = 0; i < 3; ++i) {
    text(labels[i], {680, 560 + i * 100.f});
    m_channels[i].draw(values[i], 255, tints[i]);
    text(std::to_string(values[i]), {1640, 560 + i * 100.f}, 28);
  }
  text("These colors are shared by the game's instruments.", {620, 855}, 26, theme::MutedText);
  card({600, 915, 1180, 85});
  text("NOTE DESIGNS", {630, 945}, 20, theme::Primary);
  // Notes.png is four horizontal 150px cells in this order. These previews
  // share the selected tint; viewing a design does not change chart behavior.
  const char *shapes[]{"Quarter", "Eighth", "Sixteenth", "Half"};
  for (int i = 0; i < 4; ++i) {
    const float x = 850 + i * 225.f;
    ResourceManager::drawImageRegionTo<Textures::Gameplay::Notes>(
        {150.f * i, 0, 150, 150}, {x, 915, 80, 80}, color);
    text(shapes[i], {x + 80, 946}, 21, theme::MutedText);
  }
}
bool GameplaySettings::events() {
  for (int i = 0; i < 6; ++i) {
    if (settings_ui::clicked({650 + i * 170.f, 295, 140, 160})) { reset(); m_note = i; }
  }
  const Color color = Settings::getNoteTint(m_note);
  int values[]{color.r, color.g, color.b};
  bool changed = false;
  for (int i = 0; i < 3; ++i) changed = m_channels[i].input(values[i], 255) || changed;
  if (changed) Settings::setNoteTint(m_note, {static_cast<unsigned char>(values[0]),
      static_cast<unsigned char>(values[1]), static_cast<unsigned char>(values[2]), 255});
  return changed;
}
void GameplaySettings::reset() { for (auto &slider : m_channels) slider.reset(); }
} // namespace bh
