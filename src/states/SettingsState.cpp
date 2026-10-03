#include "states/SettingsState.hpp"
#include "config/DisplaySettings.hpp"
#include "config/Settings.hpp"
#include "core/StateStack.hpp"
#include "states/MainMenuState.hpp"
#include "ui/settings/Controls.hpp"

namespace bh {
void SettingsState::draw() const noexcept {

  using namespace settings_ui;
  ResourceManager::drawImageTo<Textures::Settings::Background>({0, 0, 1920, 1080});
  
  card({60, 60, 450, 960});
  text("BAND HERO", {100, 105}, 23, theme::Primary);
  text("Settings", {100, 140}, 60);
  const char *sections[]{"General", "Custom Instruments", "Gameplay"};
  const char *descriptions[]{"Audio, performance & display", "Create and edit your instruments", "Note colors & appearance"};
  for (int i = 0; i < 3; ++i) {
    const float y = 300 + i * 110.f;
    choice({100, y, 360, 85}, "", static_cast<int>(m_section) == i);
    fittedText(sections[i], {122, y + 14, 316, 30}, 28, theme::Text);
    fittedText(descriptions[i], {122, y + 52, 316, 23}, 19, theme::MutedText);
  }
  panel({100, 744, 360, 1}, theme::Border);
  text(m_section == Section::Instruments ? "INSTRUMENT EDITOR" : "AUTO-SAVE", {100, 775}, 20, theme::Primary);
  fittedText(m_section == Section::Instruments ? "Save when you're ready." : "Changes save as you go.",
             {100, 812, 360, 30}, 24, theme::MutedText);
  m_menuButton.draw<WHITE, true, 40, TextAlign::Center>();
  switch (m_section) {
  case Section::General: m_general.draw(); break;
  case Section::Instruments: m_instruments.draw(); break;
  case Section::Gameplay: m_gameplay.draw(); break;
  }
  if (m_saveFailed) text("Could not save settings. Check folder permissions.", {620, 1005}, 24, theme::Error);
}
bool SettingsState::commit() noexcept {

  if (!m_instruments.canLeave()) return false;

  if (m_dirty) {
    const bool settingsSaved = Settings::saveSettings();
    const bool displaySaved = DisplaySettings::get().save();
    m_saveFailed = !settingsSaved || !displaySaved;
    if (!m_saveFailed) m_dirty = false;
  }
  return !m_saveFailed;
}
void SettingsState::update(float dt) noexcept { (void)dt; }

void SettingsState::events() noexcept {
  if (m_instruments.deletionPending()) {
    m_instruments.events();
    return;
  }
  const bool back = m_menuButton.updateInput(GetMousePosition());
  if (!IsWindowFocused()) {
    m_general.reset();
    m_gameplay.reset();
    m_instruments.reset();
    // Losing focus must not save instrument drafts or replace validation errors.
    if (m_dirty) {
      const bool settingsSaved = Settings::saveSettings();
      const bool displaySaved = DisplaySettings::get().save();
      m_saveFailed = !settingsSaved || !displaySaved;
      if (!m_saveFailed) m_dirty = false;
    }
    return;
  }
  for (int i = 0; i < 3; ++i) {
    if (settings_ui::clicked({100, 300 + i * 110.f, 360, 85}) && commit()) {
      m_general.reset();
      m_gameplay.reset();
      m_instruments.reset();
      m_section = static_cast<Section>(i);
      return;
    }
  }
  if (back && commit()) { m_stack.replace<MainMenuState>(); return; }
  switch (m_section) {
  case Section::General: m_dirty = m_general.events() || m_dirty; break;
  case Section::Instruments: m_instruments.events(); break;
  case Section::Gameplay: m_dirty = m_gameplay.events() || m_dirty; break;
  }
  // Save once a drag finishes, not for every intermediate slider value.
  if (m_dirty && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) commit();
  if (m_section != Section::Instruments && IsKeyPressed(KEY_ESCAPE) && commit())
    m_stack.replace<MainMenuState>();
}
void SettingsState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI, Textures::Gameplay, Textures::Settings>();
  m_general.onEnter();
  m_instruments.onEnter();
}
void SettingsState::onExit() noexcept {
  commit();
  ResourceManager::unloadTextures<Textures::UI, Textures::Gameplay, Textures::Settings>();
  m_general.reset();
  m_gameplay.reset();
  m_instruments.reset();
  m_menuButton.resetInteraction();
}
} // namespace bh
