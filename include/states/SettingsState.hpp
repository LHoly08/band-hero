#pragma once

#include <cstdint>

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class SettingsState final : public State {
public:
  inline SettingsState(StateStack &stack) noexcept
      : State(stack), m_saveButton("assets/textures/MainMenu/UI/Button.png",
                                   {.x = 500, .y = 500}),
        m_menuButton("assets/textures/MainMenu/UI/Button.png",
                     {.x = 1000, .y = 300}),
        m_defaultButton("assets/textures/MainMenu/UI/Button.png",
                        {.x = 700, .y = 100}) {}
  ~SettingsState() override = default;

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
