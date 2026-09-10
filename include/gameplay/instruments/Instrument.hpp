#pragma once

#include <cstdint>

#include <random>
#include <thread>
#include <vector>

#include "raylib.h"

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
};

template <InstrumentType Type>
  requires SectionInstrumentType<Type>
struct Note<Type> {

  std::uint32_t note{};
  float positionY{};
};

// Size: 104 | Align: 8
template <InstrumentType Type, Difficulty Dif> class Instrument {
public:
  explicit Instrument(std::uint32_t &noteCount);
  virtual ~Instrument() = default;

  inline void updateSpeed(const float *ptr) noexcept { m_speed = ptr; }

  virtual inline bool getPlay(std::uint32_t playedNote) noexcept {

    // Return false if bits that are not meant to be played are played
    if ((playedNote & (~m_playingNote)) || !m_playingNote) {
      return false;
    }

    // Update for bits that have not been played yet
    m_playingNote ^= (playedNote & m_playingNote);

    // Return true and clear needed notes if played is equal to original
    m_playingNote *= (bool)(m_originalNote ^ playedNote);

    return !m_playingNote;
  }
  virtual void draw(std::uint32_t startingPositionX) const noexcept = 0;
  virtual void update(float dt) noexcept = 0;

protected:
  inline void drawNote(const Vector2 &position,
                       const Color &tint) const noexcept {

    static std::random_device r;
    static std::default_random_engine el(r());

    static std::uniform_int_distribution<std::uint8_t> dist(1, 2);
    const std::uint8_t noteShape = dist(el);

    static Texture2D noteTexture =
        LoadTexture("assets/texture/Gameplay/Notes.png");

    DrawTextureRec(noteTexture,
                   {.x = 150 * (noteShape - 1.f),
                    .y = 0.f,
                    .width = 150.f,
                    .height = 150.f},
                   position, tint);
  }

  using NoteType = Note<Type>;

  std::uint32_t m_originalNote{};
  std::uint32_t m_playingNote{};

  std::vector<NoteType> m_activeBuffer;
  std::vector<NoteType> m_loadingBuffer;
  std::vector<NoteType> m_downloadingBuffer;
  std::thread m_loadingThread;
  std::uint32_t &m_noteCount;
  const float *m_speed;
};

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount)
    : m_noteCount(noteCount) {}

} // namespace bh
