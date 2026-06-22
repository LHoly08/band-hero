#pragma once
#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> class Guitar final : public Instrument<Dif> {
public:
  explicit Guitar(std::string filename) noexcept;

  inline constexpr std::uint8_t getNumberStrings() const noexcept {
    return NumberStrings<Dif>::Guitar;
  }

  inline bool getPlay(std::uint32_t &val) const noexcept override {
    if constexpr (Dif == Difficulty::EASY) {
      for (std::uint8_t i{}, string = (val & 0x1F);
        i < NumberStrings<Dif>::Bass;
        string = (val >> (++i * 5)) & 0x1F) {

        val |= (string != 0) * (0x1F << (i * 5));
      }
    }
    return Instrument::getPlay(val);
  }
};

template <Difficulty Dif>
Guitar<Dif>::Guitar(std::string filename) noexcept
    : Instrument<Dif>("guitar/" + filename) {}

} // namespace bh
