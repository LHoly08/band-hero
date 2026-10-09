#pragma once
#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class BuildingState final : public State {
public:
  inline BuildingState(StateStack &stack) noexcept : State(stack) {}
  ~BuildingState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<ResourceManager::TextureHandle> m_textures =
      ResourceManager::acquireTextures<Textures::UI, Textures::Building>();
  Button m_backButton{{.x = 800, .y = 850},
                      {.x = 0, .y = 600, .width = 320, .height = 96},
                      "Back"};
};

} // namespace bh
