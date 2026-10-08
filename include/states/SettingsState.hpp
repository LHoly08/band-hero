#pragma once

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/GameplaySettings.hpp"
#include "ui/settings/InstrumentSettings.hpp"

namespace bh {
class SettingsState final : public State {
public:
  explicit SettingsState(StateStack &stack) noexcept;
  ~SettingsState() override = default;
  
  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI>();
  ResourceManager::TextureHandle m_notes =
      ResourceManager::acquireTexture<Textures::Gameplay::Notes>();
  AnimatedBackground m_background;
  enum class Section { General, Instruments, Gameplay };

  bool commit() noexcept;

  Button m_menuButton{
    {100, 885},
    {0, 0, 360, 100},
    "Main Menu"
  };

  GeneralSettings m_general;
  GameplaySettings m_gameplay;
  InstrumentSettings m_instruments;
  Section m_section{Section::General};
  bool m_dirty{};
  bool m_saveFailed{};
};
} // namespace bh
