#pragma once

#include <cstdint>

#include <string>

#include "Instrument.hpp"

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

private:
  std::string m_name;
  InstrumentComposition<Type> m_composition;
};

} // namespace bh
