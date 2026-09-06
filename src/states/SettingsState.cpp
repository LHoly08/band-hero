
#include <fstream>

#include "states/SettingsState.hpp"

#include "raylib.h"

#include "core/StateStack.hpp"

#include "gameplay/Settings.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

void SettingsState::draw() const noexcept {
  m_saveButton.draw();
  m_menuButton.draw();

  switch (m_menuSection) {

  case SettingsSection::Play: {
    Settings::saveSettings();
    break;
  }
  case SettingsSection::Startup: {

    break;
  }
  case SettingsSection::Instruments: {

    break;
  }
  }
}

void SettingsState::update(float dt) noexcept {}

void SettingsState::events() noexcept {
  if (IsMouseButtonPressed(0)) {
    const Vector2 MousePos{GetMousePosition()};

    if (m_menuButton.pressed(MousePos)) {
      m_stack.replace<MainMenuState>();
    } else if (m_saveButton.pressed(MousePos)) {

      switch (m_menuSection) {

      case SettingsSection::Play: {
        Settings::saveSettings();
        break;
      }
      case SettingsSection::Startup: {
        std::fstream file;
        file.open(Settings::startupFile, std::ios::binary);

        break;
      }
      case SettingsSection::Instruments: {

        break;
      }
      }
    }
  }
}

void SettingsState::onEnter() noexcept {}

void SettingsState::onExit() noexcept {}

} // namespace bh
