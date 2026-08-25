#pragma once

#include <cstdint>

#include <string>

#include "Instrument.hpp"
#include "raylib.h"

#include "../Settings.hpp"

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

// Size: 136 | Align: 8
template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
class Custom final : public Instrument<Type, Dif> {
public:
  using Base = Instrument<Type, Dif>;

  inline explicit Custom(std::uint32_t &noteCount) : Base(noteCount) {}
  ~Custom() override = default;

  inline bool getPlay(std::uint32_t playedNote) noexcept override;

  void draw(std::uint32_t startingPositionX) const noexcept override;
  void update(float dt) noexcept override;

private:
  std::string m_name;
  InstrumentComposition<Type> m_composition;
};

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
void Custom<Type, Dif>::draw(std::uint32_t startingPositionX) const noexcept {

  if constexpr (Type == InstrumentType::Custom_1) {

    static Texture2D noteTexture =
        LoadTexture("assets/texture/Gameplay/Note.png");

    for (const auto &note : this->m_activeBuffer) {
      for (std::uint8_t i{}; i < m_composition.NumberSections; ++i) {

        std::uint8_t fretVal =
            (note.note >> (i * m_composition.NumberBitsSection)) &
            m_composition.NumberBitsSection;

        if (fretVal) {
          DrawTexture(noteTexture, startingPositionX + fretVal * 50,
                      note.positionY, Settings::getNoteTint(note.stringNumber));
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
