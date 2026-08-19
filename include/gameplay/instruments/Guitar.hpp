#pragma once

#include <cstdint>

#include "Instrument.hpp"

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

  inline explicit Guitar(std::uint32_t &noteCount) : Base(noteCount) {}
  ~Guitar() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override {

    playedNote &= ((1 << (GuitarComposition<Dif>::FretBits *
                          GuitarComposition<Dif>::Strings)) -
                   1);

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

  void draw() const noexcept override;
  void update(float dt) noexcept override;

private:
};

template <Difficulty Dif> void Guitar<Dif>::draw() const noexcept {}

template <Difficulty Dif> void Guitar<Dif>::update(float dt) noexcept {}

} // namespace bh
