#include "core/AnimationPlayer.hpp"

#include <algorithm>
#include <cmath>
#include "rlgl.h"

#include "core/Scale.hpp"

namespace bh {

AnimationPlayer::AnimationPlayer(Clip clip) noexcept : m_clip(clip) {
  m_clip.columns = std::max(1, clip.columns);
  m_clip.rows = std::max(1, clip.rows);
  const auto cells = static_cast<long long>(m_clip.columns) * m_clip.rows;
  m_clip.frameCount = static_cast<int>(
      std::clamp(static_cast<long long>(clip.frameCount), 1LL, cells));
  if (!std::isfinite(clip.framesPerSecond) || clip.framesPerSecond <= 0.f)
    m_clip.framesPerSecond = 12.f;
}

void AnimationPlayer::update(float dt) noexcept {
  if (!m_playing || !std::isfinite(dt) || dt <= 0.f) return;

  if (m_clip.frameCount <= 1) return;
  const double duration = m_clip.frameCount / double(m_clip.framesPerSecond);
  m_frameTime += dt;
  if (m_clip.loop) {
    m_frameTime = std::fmod(m_frameTime, duration);
  } else if (m_frameTime >= duration) {
    m_frameTime = duration;
    m_frame = m_clip.frameCount - 1;
    m_playing = false;
    return;
  }
  m_frame = std::min(m_clip.frameCount - 1,
                     static_cast<int>(m_frameTime * m_clip.framesPerSecond));
}

void AnimationPlayer::reset() noexcept {
  m_frameTime = 0;
  m_frame = 0;
  m_playing = true;
}

Rectangle AnimationPlayer::sourceRectangle(Texture2D texture) const noexcept {
  return sourceRectangle(texture, m_frame);
}

int AnimationPlayer::nextFrame() const noexcept {
  if (m_frame + 1 < m_clip.frameCount) return m_frame + 1;
  return m_clip.loop ? 0 : m_frame;
}

float AnimationPlayer::frameBlend() const noexcept {
  if (!m_clip.interpolate || nextFrame() == m_frame) return 0.f;
  const double progress = m_frameTime * m_clip.framesPerSecond;
  return static_cast<float>(progress - std::floor(progress));
}

Rectangle AnimationPlayer::sourceRectangle(Texture2D texture, int frame) const noexcept {
  const float width = float(texture.width) / m_clip.columns;
  const float height = float(texture.height) / m_clip.rows;
  return {(frame % m_clip.columns) * width,
          (frame / m_clip.columns) * height, width, height};
}

void AnimationPlayer::draw(Texture2D texture, const Rectangle &destination,
                           Color tint, float rotation) const noexcept {
  if (!IsTextureValid(texture) || destination.width <= 0.f ||
      destination.height <= 0.f) return;
  const auto position = scaledSize(Vector2{destination.x, destination.y});
  const auto scale = scaledSize(Vector2{1, 1});
  const float blend = frameBlend();
  const auto weightedTint = [&](float weight) {
    Color result = tint;
    result.a = static_cast<unsigned char>(std::lround(tint.a * weight));
    return result;
  };
  if (m_clip.additive) BeginBlendMode(BLEND_ADDITIVE);
  // Scale the rotated geometry in layout space so the side strip stays aligned
  // to the edge even when the window's X/Y scales differ.
  rlPushMatrix();
  rlTranslatef(position.x, position.y, 0);
  rlScalef(scale.x, scale.y, 1);
  const Rectangle target{0, 0, destination.width, destination.height};
  DrawTexturePro(texture, sourceRectangle(texture), target, {0, 0}, rotation,
                 m_clip.additive ? weightedTint(1.f - blend) : tint);
  if (blend > 0.f) {
    DrawTexturePro(texture, sourceRectangle(texture, nextFrame()),
                   target, {0, 0}, rotation, weightedTint(blend));
  }
  rlPopMatrix();
  if (m_clip.additive) EndBlendMode();
}

} // namespace bh
