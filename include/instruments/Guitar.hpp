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
};

template <Difficulty Dif>
Guitar<Dif>::Guitar(std::string filename) noexcept
    : Instrument<Dif>("guitar/" + filename) {}

} // namespace bh
