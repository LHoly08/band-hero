#pragma once

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"
#include "ui/Button.hpp"
#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/GameplaySettings.hpp"
#include "ui/settings/InstrumentSettings.hpp"

namespace bh {
class SettingsState final : public bh::State {
public:
  explicit SettingsState(bh::StateStack &stack) noexcept;
  ~SettingsState() override = default;
  
  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI>();
  bh::ResourceManager::TextureHandle m_notes =
      bh::ResourceManager::acquireTexture<bh::Textures::Gameplay::Notes>();
  bh::AnimatedBackground m_background;
  enum class Section { General, Instruments, Gameplay };

  bool commit() noexcept;

  bh::Button m_menuButton{
    {100, 885},
    {0, 0, 360, 100},
    "Main Menu"
  };

  bh::GeneralSettings m_general;
  bh::GameplaySettings m_gameplay;
  bh::InstrumentSettings m_instruments;
  Section m_section{Section::General};
  bool m_dirty{};
  bool m_saveFailed{};
};
} // namespace bh
