#pragma once

#include "raylib.h"

namespace bh {

// Plays a row-major sprite sheet without owning its texture. ResourceManager
// retains ownership, so unloading/reloading a screen cannot leave a stale ID.
class AnimationPlayer {
public:
  struct Clip {
    int columns{1};
    int rows{1};
    int frameCount{1};
    float framesPerSecond{12.f};
    bool loop{true};
    // Crossfade opaque frames using fractional playback time.
    bool interpolate{false};
    // Alpha-weighted additive blending supports transparent neon overlays.
    bool additive{false};
  };

  AnimationPlayer() noexcept = default;
  explicit AnimationPlayer(Clip clip) noexcept;

  void update(float dt) noexcept;
  void play() noexcept { m_playing = true; }
  void pause() noexcept { m_playing = false; }
  void reset() noexcept;
  int frame() const noexcept { return m_frame; }
  bool isPlaying() const noexcept { return m_playing; }
  int nextFrame() const noexcept;
  float frameBlend() const noexcept;

  Rectangle sourceRectangle(Texture2D texture) const noexcept;
  void draw(Texture2D texture, const Rectangle &destination,
            Color tint = WHITE, float rotation = 0.f) const noexcept;

private:
  Rectangle sourceRectangle(Texture2D texture, int frame) const noexcept;
  Clip m_clip{};
  double m_frameTime{};
  int m_frame{};
  bool m_playing{true};
};

} // namespace bh
