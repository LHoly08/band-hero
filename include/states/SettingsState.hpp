#pragma once

#include <cstdint>

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class SettingsState final : public State {
public:
  inline SettingsState(StateStack &stack) noexcept
      : State(stack),
        m_saveButton({.x = 500, .y = 600}, {0, 0, 500, 200}, "Save"),
        m_menuButton({.x = 1000, .y = 350}, {0, 0, 500, 200}, "Main Menu"),
        m_defaultButton({.x = 700, .y = 100}, {0, 0, 500, 200}, "Defaults") {}
  ~SettingsState() override {
    ResourceManager::unloadTextures<Textures::UI>();
  };

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  enum class SettingsSection : std::uint8_t {
    Startup = 0,
    Play,
    Instruments,
  };

  Button m_saveButton;
  Button m_menuButton;
  Button m_defaultButton;

  SettingsSection m_menuSection{SettingsSection::Startup};
};

} // namespace bh
