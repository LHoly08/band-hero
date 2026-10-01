
#include <fstream>

#include "states/SettingsState.hpp"

#include "raylib.h"

#include "config/Settings.hpp"

#include "core/StateStack.hpp"

#include "states/MainMenuState.hpp"

namespace bh {

void SettingsState::draw() const noexcept {
  m_saveButton.draw();
  m_menuButton.draw();
  m_defaultButton.draw();

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

void SettingsState::update(float dt) noexcept { auto _ = dt; }

void SettingsState::events() noexcept {
  const Vector2 mousePos = GetMousePosition();
  const bool menuClicked = m_menuButton.updateInput(mousePos);
  const bool saveClicked = m_saveButton.updateInput(mousePos);
  m_defaultButton.updateInput(mousePos);
  if (menuClicked || saveClicked) {

    if (menuClicked) [[unlikely]] {
      m_stack.replace<MainMenuState>();
    } else if (saveClicked) [[unlikely]] {

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

void SettingsState::onEnter() noexcept {
  ResourceManager::loadTextures<Textures::UI>();
}

void SettingsState::onExit() noexcept {
  m_menuButton.resetInteraction();
  m_saveButton.resetInteraction();
  m_defaultButton.resetInteraction();
}

} // namespace bh
