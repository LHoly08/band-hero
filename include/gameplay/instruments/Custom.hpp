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

  explicit Custom(std::uint32_t &noteCount, std::string filename,
                  std::string_view instrumentName,
                  InstrumentComposition<Type> instrumentComposition);
  ~Custom() override = default;

  bool getPlay(std::uint32_t playedNote) noexcept override {
    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;

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
Custom<Type, Dif>::Custom(std::uint32_t &noteCount, std::string filename,
                          std::string_view instrumentName,
                          InstrumentComposition<Type> instrumentComposition)
    : Base(
          noteCount,
          std::move(filename.append([this]() consteval -> auto {
  constexpr auto self = std::meta::remove_cvref(^^decltype(*this));

  std::string path{};

  static constexpr auto templateArgs =
      std::define_static_array(std::meta::template_arguments_of(self));

  template for (constexpr auto arg : templateArgs) {
    using T = [:std::meta::type_of(arg):];

    if constexpr (std::meta::is_enum_type(^^T)) {

      static constexpr auto enumerators = std::define_static_array(
          std::meta::enumerators_of(std::meta::dealias(^^T)));

      template for (constexpr auto enumerator : enumerators) {
        if constexpr (std::meta::extract<T>(enumerator) ==
                      std::meta::extract<T>(arg)) {

          path.append("/");
          path.append(std::meta::identifier_of(enumerator));
        }
      }
    }
  }
  path.append("/");

  return std::define_static_string(path);
          }()).append(instrumentName).append(".file"))),
      m_name(instrumentName), m_composition(instrumentComposition) {}

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
void Custom<Type, Dif>::draw(std::uint32_t startingPositionX) const noexcept {
  if constexpr (Type == InstrumentType::Custom_1) {

    for (const auto &note : this->m_activeBuffer) {
      if (note.timeStamp - this->m_time > 2.f) {
        break;
      }
      const auto bits = m_composition.NumberBitsSection;
      if (bits == 0 || bits > 30) {
        continue;
      }
      const auto sections =
          std::min<unsigned>(m_composition.NumberSections, 30 / bits);
      for (std::uint8_t i{}; i < sections; ++i) {

        std::uint32_t fretVal = (note.note >> (i * bits)) & ((1u << bits) - 1u);

        if (fretVal) [[unlikely]] {

          this->drawNote(
              {.x = static_cast<float>(startingPositionX + fretVal * 50),
               .y = static_cast<float>((note.timeStamp - this->m_time) * 5 +
                                       (OriginalWindowSize.y - 30))},
              Settings::getNoteTint(i), note.shape);
        }
      }
    }

  } else if constexpr (Type == InstrumentType::Custom_2) {

    for (const auto &note : this->m_activeBuffer) {
      if (note.timeStamp - this->m_time > 2.f) {
        break;
      }
      const auto bits = Dif == Difficulty::Easy
                            ? m_composition.NumberEffectiveBitsEasy
                            : m_composition.NumberEffectiveBitsHard;
      for (std::uint8_t i{}; i < std::min<unsigned>(bits, 30); ++i) {

        if (bool playedBit = (note.note >> i) & 1; playedBit) [[unlikely]] {

          this->drawNote({.x = static_cast<float>(startingPositionX + i * 50),
                          .y = static_cast<float>((note.timeStamp - this->m_time) * 5 +
                                                  (OriginalWindowSize.y - 30))},
                         Settings::getNoteTint(i), note.shape);
        }
      }
    }
  }
}

} // namespace bh
