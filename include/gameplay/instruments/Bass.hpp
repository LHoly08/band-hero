#pragma once

#include <cstdint>

#include "Instrument.hpp"
#include "raylib.h"

#include "../Settings.hpp"

namespace bh {

template <Difficulty Dif> struct BassComposition;

template <> struct BassComposition<Difficulty::Easy> {
  enum : std::uint8_t {
    FretBits = 5,
    Strings = 4,
  };
};

template <> struct BassComposition<Difficulty::Hard> {
  enum : std::uint8_t {
    FretBits = 5,
    Strings = 4,
  };
};

template <Difficulty Dif>
class Bass final : public Instrument<InstrumentType::Bass, Dif> {
public:
  using Base = Instrument<InstrumentType::Bass, Dif>;

  inline explicit Bass(std::uint32_t &noteCount) : Base(noteCount) {}
  ~Bass() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override {

    playedNote &= ((1 << (BassComposition<Dif>::FretBits *
                          BassComposition<Dif>::Strings)) -
                   1);

    if constexpr (Dif == Difficulty::Easy) {

      for (std::uint8_t i{}; i < BassComposition<Dif>::Strings; ++i) {

        constexpr std::uint8_t shiftAmount{BassComposition<Dif>::FretBits};

        // Creation of x bits all set to 1
        constexpr std::uint8_t fullBits =
            (1 << BassComposition<Dif>::FretBits) - 1;

        // Get if string was played
        bool played = (playedNote >> (i * shiftAmount)) & fullBits;

        // Set every bit of the string to 1 if played
        playedNote |= played * (fullBits << (i * shiftAmount));
      }
    }
    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;
  void update(float dt) noexcept override;

private:
};

template <Difficulty Dif>
void Bass<Dif>::draw(std::uint32_t startingPositionX) const noexcept {
  static Texture2D noteTexture =
      LoadTexture("assets/texture/Gameplay/Note.png");

  for (const auto &note : this->m_activeBuffer) {
    for (std::uint8_t i{}; i < BassComposition<Dif>::Strings; ++i) {

      std::uint8_t fretVal =
          (note.note >> (i * BassComposition<Dif>::FretBits)) &
          BassComposition<Dif>::FretBits;

      if (fretVal) {
        DrawTexture(noteTexture, startingPositionX + fretVal * 50,
                    note.positionY, Settings::getNoteTint(i));
      }
    }
  }
}

template <Difficulty Dif> void Bass<Dif>::update(float dt) noexcept {}

} // namespace bh
