#pragma once

#include <cstdint>

#include <random>
#include <string>
#include <string_view>
#include <variant>

#include "raylib.h"

#include "core/Scale.hpp"

#include "gameplay/Settings.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

template <InstrumentType Type>
concept CustomType =
    Type == InstrumentType::Custom_1 || Type == InstrumentType::Custom_2;

template <InstrumentType Type>
  requires CustomType<Type>
struct InstrumentComposition;

template <> struct InstrumentComposition<InstrumentType::Custom_1> {
  std::uint8_t NumberSections{};
  std::uint8_t NumberBitsSection{};
};

template <> struct InstrumentComposition<InstrumentType::Custom_2> {
  std::uint8_t NumberEffectiveBitsEasy{};
  std::uint8_t NumberEffectiveBitsHard{};
};

struct CustomInstrumentComposition {
  std::string name;
  std::variant<InstrumentComposition<InstrumentType::Custom_1>,
               InstrumentComposition<InstrumentType::Custom_2>>
      composition;
};

// Size: 136 | Align: 8
template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
class Custom final : public Instrument<Type, Dif> {
public:
  using Base = Instrument<Type, Dif>;

  explicit Custom(std::uint32_t &noteCount, std::string_view instrumentName,
                  InstrumentComposition<Type> instrumentComposition);
  ~Custom() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override {
    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;
  void update(float dt) noexcept override;

private:
  std::string m_name;
  InstrumentComposition<Type> m_composition;
};

template <>
bool Custom<InstrumentType::Custom_1, Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<InstrumentType::Custom_1, Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<InstrumentType::Custom_2, Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<InstrumentType::Custom_2, Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept;

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
Custom<Type, Dif>::Custom(std::uint32_t &noteCount,
                          std::string_view instrumentName,
                          InstrumentComposition<Type> instrumentComposition)
    : Base(noteCount), m_name(instrumentName),
      m_composition(instrumentComposition) {}

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
void Custom<Type, Dif>::draw(std::uint32_t startingPositionX) const noexcept {

  static std::random_device r;
  static std::default_random_engine el(r());

  if constexpr (Type == InstrumentType::Custom_1) {

    for (const auto &note : this->m_activeBuffer) {
      for (std::uint8_t i{}; i < m_composition.NumberSections; ++i) {

        std::uint8_t fretVal =
            (note.note >> (i * m_composition.NumberBitsSection)) &
            m_composition.NumberBitsSection;

        if (fretVal) {

          this->drawNote(
              {.x = static_cast<float>(startingPositionX + fretVal * 50),
               .y = note.positionY},
              Settings::getNoteTint(i));
        }
      }
    }

  } else if constexpr (Type == InstrumentType::Custom_2) {
  }
}

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
void Custom<Type, Dif>::update(float dt) noexcept {}

} // namespace bh
