#pragma once
#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> class Bass final : public Instrument<Dif> {
public:
  explicit Bass(std::string filename) noexcept;

  inline constexpr std::uint8_t getNumberStrings() const noexcept {
    return NumberStrings<Dif>::Bass;
  }
};

template <Difficulty Dif>
Bass<Dif>::Bass(std::string filename) noexcept
    : Instrument<Dif>("bass/" + filename) {}

} // namespace bh
