#pragma once

#include <array>

#include "states/State.hpp"
#include "ui/AnimatedBackground.hpp"

#include "ui/Button.hpp"

namespace bh {

class GuidesState final : public State {
public:
  inline GuidesState(StateStack &stack) noexcept
      : State(stack) {}
  ~GuidesState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI>();
  AnimatedBackground m_background;

  Button m_mainMenuButton{
    {.x = 1350, .y = 30},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Main Menu"
  };
  Button m_firstGuideButton{
    {.x = 195, .y = 30},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Bass Guide"
  };
  Button m_secondGuideButton{
    {.x = 580, .y = 30},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Drums Guide"
  };
  Button m_thirdGuideButton{
    {.x = 965, .y = 30},
    {.x = 0, .y = 0, .width = 360, .height = 100},
    "Guitar Guide"
  };

  enum class Guide : std::uint8_t {
    Bass = 0,
    Drums,
    Guitar,
    Hub,
  };

  Guide m_currentGuide{Guide::Hub};
  std::array<Guide, 3> m_buttonGuides{Guide::Bass, Guide::Drums, Guide::Guitar};

  void changeGuide(Guide newGuide) noexcept;
};

} // namespace bh
