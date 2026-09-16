#pragma once

#include <cstdint>
#include <thread>
#include <vector>

#include "raylib.h"

#include "core/ResourceManager.hpp"

namespace bh {

enum class Difficulty : std::uint8_t {
  Easy = 0,
  Hard,
};

enum class InstrumentType : std::uint8_t {
  Bass = 0,
  Drums,
  Guitar,
  Custom_1,
  Custom_2,
};

template <InstrumentType Type>
concept SectionInstrumentType =
    Type != InstrumentType::Drums && Type != InstrumentType::Custom_2;

template <InstrumentType Type> struct Note {

  std::uint32_t note{};
  float positionY{};
  std::uint8_t shape{static_cast<std::uint8_t>(GetRandomValue(0, 1))};
};

template <InstrumentType Type>
  requires SectionInstrumentType<Type>
struct Note<Type> {

  std::uint32_t note{};
  float positionY{};
  std::uint8_t shape{static_cast<std::uint8_t>(GetRandomValue(0, 1))};
};

// Size: 104 | Align: 8
template <InstrumentType Type, Difficulty Dif> class Instrument {
public:
  explicit Instrument(std::uint32_t &noteCount);
  virtual ~Instrument() = default;

  inline void updateSpeed(const float &speed) noexcept { m_speed = speed; }

  virtual bool getPlay(std::uint32_t playedNote) noexcept;

  virtual void draw(std::uint32_t startingPositionX) const noexcept = 0;
  virtual void update(float dt) noexcept = 0;

protected:
  // Position uses reference layout coordinates; drawImage applies screen
  // scaling.
  inline void drawNote(const Vector2 &position,
                       const Color &tint, std::uint8_t shape) const noexcept {

    // Shape belongs to the note; drawing must not randomize it every frame.
    ResourceManager::drawImage<Textures::Gameplay::Notes>(
        {.x = 150.f * (shape % 2), .y = 0, .width = 150, .height = 150}, position,
        tint);
  }

  using NoteType = Note<Type>;

  std::uint32_t m_originalNote{};
  std::uint32_t m_playingNote{};

  std::vector<NoteType> m_activeBuffer;
  std::vector<NoteType> m_loadingBuffer;
  std::vector<NoteType> m_downloadingBuffer;
  std::thread m_loadingThread;
  std::uint32_t &m_noteCount;
  float m_speed{};
};

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount)
    : m_noteCount(noteCount) {}

template <InstrumentType Type, Difficulty Dif>
bool Instrument<Type, Dif>::getPlay(std::uint32_t playedNote) noexcept {

  // Return false if bits that are not meant to be played are played
  // or if nothing was played
  if ((playedNote & (~m_playingNote)) || !m_playingNote) {
    return false;
  }

  // Update for bits that have not been played yet
  m_playingNote ^= (playedNote & m_playingNote);

  // Return true and clear needed notes if played is equal to original
  m_playingNote *= static_cast<bool>(m_originalNote ^ playedNote);

  return !m_playingNote;
}

} // namespace bh
