#pragma once

#include <cstdint>

#include "core/Scale.hpp"

#include "gameplay/Settings.hpp"

#include "gameplay/instruments/Instrument.hpp"

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

    playedNote &= [] consteval -> std::uint32_t {
      std::uint8_t NumberBits = std::min(
          DrumsComposition<Dif>::Pedals + DrumsComposition<Dif>::DrumsCymbals,
          30);

      return (1 << NumberBits) - 1;
    }();

    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;
  void update(float dt) noexcept override;

private:
};

template <Difficulty Dif>
void Drums<Dif>::draw(std::uint32_t startingPositionX) const noexcept {

  for (const auto &note : this->m_activeBuffer) {

    constexpr std::uint8_t NumberBits = std::min(
        DrumsComposition<Dif>::Pedals + DrumsComposition<Dif>::DrumsCymbals,
        30);

    for (std::uint8_t i{}; i < NumberBits; ++i) {

      if (std::uint8_t playedBit = (note.note >> i) & 1; playedBit)
          [[unlikely]] {

        this->drawNote({.x = static_cast<float>(startingPositionX + i * 50),
                        .y = note.positionY},
                       Settings::getNoteTint(i));
      }
    }
  }
}

template <Difficulty Dif> void Drums<Dif>::update(float dt) noexcept {

  for (auto &note : this->m_activeBuffer) {
    note.positionY -= this->m_speed * dt;
  }
}

} // namespace bh
