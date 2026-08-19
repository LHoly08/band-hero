#pragma once

#include <cstdint>

#include "Instrument.hpp"

namespace bh {

template <Difficulty Dif> struct DrumsComposition;

template <> struct DrumsComposition<Difficulty::Easy> {
  enum : std::uint8_t {
    Pedals = 1,
    DrumsCymbals = 5,
  };
};

template <> struct DrumsComposition<Difficulty::Hard> {
  enum : std::uint8_t {
    Pedals = 2,
    DrumsCymbals = 7,
  };
};

template <Difficulty Dif>
class Drums final : public Instrument<InstrumentType::Drums, Dif> {
public:
  using Base = Instrument<InstrumentType::Drums, Dif>;

  inline explicit Drums(std::uint32_t &noteCount) : Base(noteCount) {}
  ~Drums() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override {

    constexpr std::uint8_t NumberBits{DrumsComposition<Dif>::Pedals +
                                      DrumsComposition<Dif>::DrumsCymbals};

    constexpr std::uint32_t UsedBits = (1 << NumberBits) - 1;

    playedNote &= UsedBits;
    return Base::getPlay(playedNote);
  }

  void draw() const noexcept override;
  void update(float dt) noexcept override;

private:
};

template <Difficulty Dif> void Drums<Dif>::draw() const noexcept {}

template <Difficulty Dif> void Drums<Dif>::update(float dt) noexcept {}

} // namespace bh
