#pragma once

#include <cstdint>

#include "instruments/Instrument.hpp"

namespace bh {

template <Difficulty Dif> struct DrumsNums;

template <> struct DrumsNums<Difficulty::EASY> {
  enum {
    Pedals = 1,
    Pads = 3,
  };
};

template <> struct DrumsNums<Difficulty::HARD> {
  enum {
    Pedals = 2,
    Pads = 6,
  };
};

template <Difficulty Dif> class Drums final : public Instrument<Dif> {
public:
  explicit Drums(std::string filename) noexcept;

  inline constexpr std::uint8_t getNumberPads() const noexcept {
    return DrumsNums<Dif>::Pads;
  }
  inline constexpr std::uint8_t getNumberPedals() const noexcept {
    return DrumsNums<Dif>::Pedals;
  }
};

template <Difficulty Dif>
Drums<Dif>::Drums(std::string filename) noexcept
    : Instrument<Dif>("drums/" + filename) {}

} // namespace bh
