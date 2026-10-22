#include "states/SettingsState.hpp"

#include "config/DisplaySettings.hpp"
#include "config/Settings.hpp"

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

#include "ui/settings/Controls.hpp"

namespace bh {
void bh::SettingsState::draw() const noexcept {

  m_background.draw();

  bh::settings_ui::card({60, 60, 450, 960});
  bh::settings_ui::text("BAND HERO", {100, 105}, 23, bh::theme::Primary);
  bh::settings_ui::text("Settings", {100, 140}, 60);
  const char *sections[]{"General", "Custom Instruments", "Gameplay"};
  const char *descriptions[]{"Audio, display & serial controller",
                             "Create and edit your instruments",
                             "Note colors & appearance"};
  for (int i = 0; i < 3; ++i) {
    const float y = 300 + i * 110.f;
    bh::settings_ui::choice({100, y, 360, 85}, "", static_cast<int>(m_section) == i);
    bh::settings_ui::fittedText(sections[i], {122, y + 14, 316, 30}, 28, bh::theme::Text);
    bh::settings_ui::fittedText(descriptions[i], {122, y + 52, 316, 23}, 19, bh::theme::MutedText);
  }
  bh::settings_ui::panel({100, 744, 360, 1}, bh::theme::Border);
  bh::settings_ui::text(m_section == Section::Instruments ? "INSTRUMENT EDITOR" : "AUTO-SAVE",
       {100, 775}, 20, bh::theme::Primary);
  bh::settings_ui::fittedText(m_section == Section::Instruments
                 ? "Leaving discards unsaved edits."
                 : "Changes save as you go.",
             {100, 812, 360, 30}, 24, bh::theme::MutedText);
  m_menuButton.draw<WHITE, true, 40, bh::TextAlign::Center>();
  switch (m_section) {
  case Section::General:
    m_general.draw();
    break;
  case Section::Instruments:
    m_instruments.draw();
    break;
  case Section::Gameplay:
    m_gameplay.draw();
    break;
  }
  if (m_saveFailed)
    bh::settings_ui::text("Could not save settings. Check folder permissions.", {620, 1005}, 24,
         bh::theme::Error);
}
bool bh::SettingsState::commit() noexcept {
  // General/Gameplay changes are live and auto-saved. Instrument drafts have
  // their own explicit Save action and must never be persisted by this method.

  if (m_dirty) {
    const bool settingsSaved = bh::Settings::saveSettings();
    const bool displaySaved = bh::DisplaySettings::get().save();
    m_saveFailed = !settingsSaved || !displaySaved;
    if (!m_saveFailed)
      m_dirty = false;
  }
  return !m_saveFailed;
}
void bh::SettingsState::update(float dt) noexcept { m_background.update(dt); }

void bh::SettingsState::events() noexcept {
  if (m_instruments.deletionPending()) {
    m_instruments.events();
    return;
  }
  const bool back = m_menuButton.updateInput(::GetMousePosition());
  if (!::IsWindowFocused()) {
    m_general.reset();
    m_gameplay.reset();
    m_instruments.reset();
    // Losing focus must not save instrument drafts or replace validation
    // errors.
    if (m_dirty) {
      const bool settingsSaved = bh::Settings::saveSettings();
      const bool displaySaved = bh::DisplaySettings::get().save();
      m_saveFailed = !settingsSaved || !displaySaved;
      if (!m_saveFailed)
        m_dirty = false;
    }
    return;
  }
  for (int i = 0; i < 3; ++i) {
    if (i != static_cast<int>(m_section) &&
        bh::settings_ui::clicked({100, 300 + i * 110.f, 360, 85}) && commit()) {
      // Only leaving the section discards drafts; clicking its active tab does
      // not. Reload from disk so returning to the editor shows saved values.
      if (m_section == Section::Instruments)
        m_instruments.discard();
      m_general.reset();
      m_gameplay.reset();
      m_instruments.reset();
      m_section = static_cast<Section>(i);
      return;
    }
  }
  if (back && commit()) {
    if (m_section == Section::Instruments)
      m_instruments.discard();
    m_stack.pop();
    return;
  }
  switch (m_section) {
  case Section::General:
    m_dirty = m_general.events() || m_dirty;
    break;
  case Section::Instruments:
    m_instruments.events();
    break;
  case Section::Gameplay:
    m_dirty = m_gameplay.events() || m_dirty;
    break;
  }
  // Save once a drag finishes, not for every intermediate slider value.
  if (m_dirty && !::IsMouseButtonDown(::MOUSE_BUTTON_LEFT))
    commit();
  if (m_section != Section::Instruments && ::IsKeyPressed(::KEY_ESCAPE) && commit())
    m_stack.replace<bh::MainMenuState>();
}
bh::SettingsState::SettingsState(bh::StateStack &stack) noexcept : bh::State(stack) {
  m_general.onEnter();
  m_instruments.onEnter();
}
void bh::SettingsState::onEnter() noexcept {}
void bh::SettingsState::onExit() noexcept {
  commit();
  m_general.reset();
  m_gameplay.reset();
  m_instruments.reset();
  m_menuButton.resetInteraction();
}
} // namespace bh
