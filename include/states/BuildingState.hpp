#pragma once
#include "core/ResourceManager.hpp"

#include "states/State.hpp"

#include "ui/Button.hpp"

namespace bh {

class BuildingState final : public bh::State {
public:
  inline BuildingState(bh::StateStack &stack) noexcept : bh::State(stack) {}
  ~BuildingState() override = default;

  void draw() const noexcept override;
  void update(float dt) noexcept override;
  void events() noexcept override;
  void onEnter() noexcept override;
  void onExit() noexcept override;

private:
  std::vector<bh::ResourceManager::TextureHandle> m_textures =
      bh::ResourceManager::acquireTextures<bh::Textures::UI, bh::Textures::Building>();
  bh::Button m_backButton{{.x = 800, .y = 850},
                      {.x = 0, .y = 600, .width = 320, .height = 96},
                      "Back"};
};

} // namespace bh
