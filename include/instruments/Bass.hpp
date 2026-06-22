#pragma once
#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> class Bass final : public Instrument<Dif> {
public:
  explicit Bass(std::string filename) noexcept;

  inline bool getPlay(std::uint32_t &val) const noexcept {
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
Bass<Dif>::Bass(std::string filename) noexcept
    : Instrument<Dif>("bass/" + filename) {}

} // namespace bh
