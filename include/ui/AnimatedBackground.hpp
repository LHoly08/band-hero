#pragma once

#include "core/AnimationPlayer.hpp"
#include "core/ResourceManager.hpp"

namespace bh {

// Shared background for screens that draw the animated menu waves.
class AnimatedBackground {
public:
  void update(float dt) noexcept {
    m_backgroundAnimation.update(dt);
  }

  void draw() const noexcept {
    ResourceManager::drawImageTo<Textures::UI::Background>({0, 0, 1920, 1080});
    const auto waves = ResourceManager::texture<Textures::UI::Waves>();
    m_backgroundAnimation.draw(waves, {0, 952, 1920, 128});
    m_backgroundAnimation.draw(waves, {1792, 1080, 1080, 128}, WHITE, -90.f);
  }

private:
  // Full-width transparent edge strips, smoothly blended over a static backdrop.
  AnimationPlayer m_backgroundAnimation{
      AnimationPlayer::Clip{2, 30, 60, 15.f, true, true, true}};
};

} // namespace bh
