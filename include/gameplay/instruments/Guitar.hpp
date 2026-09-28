#pragma once

#include <cstdint>

#include "raylib.h"

#include "core/Scale.hpp"

#include "config/Settings.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> struct GuitarComposition;

template <> struct GuitarComposition<Difficulty::Easy> {
  enum : std::uint8_t {
    FretBits = 5,
    Strings = 6,
  };
};

template <> struct GuitarComposition<Difficulty::Hard> {
  enum : std::uint8_t {
    FretBits = 5,
    Strings = 6,
  };
};

template <Difficulty Dif>
class Guitar final : public Instrument<InstrumentType::Guitar, Dif> {
public:
  using Base = Instrument<InstrumentType::Guitar, Dif>;

  inline explicit Guitar(std::uint32_t &noteCount, std::string filename)
      : Base(noteCount, std::move(filename)) {}
  ~Guitar() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override {

    playedNote &= [] consteval {
      std::uint8_t GuitarBits = std::min(GuitarComposition<Dif>::FretBits *
                                             GuitarComposition<Dif>::Strings,
                                         30);

      return ((1 << GuitarBits) - 1);
    }();

    if constexpr (Dif == Difficulty::Easy) {

      for (std::uint8_t i{}; i < GuitarComposition<Dif>::Strings; ++i) {

        constexpr std::uint8_t shiftAmount{GuitarComposition<Dif>::FretBits};

        // Creation of x bits all set to 1
        constexpr std::uint8_t fullBits =
            (1 << GuitarComposition<Dif>::FretBits) - 1;

        // Get if string was played
        bool played = (playedNote >> (i * shiftAmount)) & fullBits;

        // Set every bit of the string to 1 if played
        playedNote |= played * (fullBits << (i * shiftAmount));
      }
    }
    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;

private:
};

template <Difficulty Dif>
void Guitar<Dif>::draw(std::uint32_t startingPositionX) const noexcept {

  for (const auto &note : this->m_activeBuffer) {
    if (note.timeStamp - this->m_time > 2.f) {
      continue;
    }

    for (std::uint8_t i{}; i < GuitarComposition<Dif>::Strings; ++i) {

      std::uint8_t fretVal =
          (note.note >> (i * GuitarComposition<Dif>::FretBits)) &
          ((1u << GuitarComposition<Dif>::FretBits) - 1u);

      if (fretVal) [[unlikely]] {
        const float posY =
            (note.timeStamp - this->m_time) * 5 + (OriginalWindowSize.y - 30);

        this->drawNote(
            {.x = static_cast<float>(startingPositionX + fretVal * 50),
             .y = posY},
            Settings::getNoteTint(i), note.shape);
      }
    }
  }
}

} // namespace bh
