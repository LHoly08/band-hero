#pragma once

#include <lua.hpp>

#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <variant>

#include "raylib.h"

#include "config/Settings.hpp"

#include "core/Scale.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

struct CustomInstrumentComposition {
  std::string name;
  std::variant<InstrumentComposition<InstrumentType::Custom_1>,
               InstrumentComposition<InstrumentType::Custom_2>,
               InstrumentComposition<InstrumentType::Custom_3>>
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

template <Difficulty Dif>
class Custom<InstrumentType::Custom_3, Dif> final
    : public Instrument<InstrumentType::Custom_3, Dif> {
public:
  using Base = Instrument<InstrumentType::Custom_3, Dif>;

  explicit Custom(
      std::uint32_t &noteCount, std::string filename,
      std::string_view instrumentName,
      InstrumentComposition<InstrumentType::Custom_3> instrumentComposition);

  ~Custom() override {
    if (m_lua) {
      lua_close(m_lua);
    }
  };

  bool getPlay(std::uint32_t playedNote) noexcept override;

  void draw(std::uint32_t startingPositionX) const noexcept override;

private:
  std::string m_name;
  lua_State *m_lua;
};

template <Difficulty Dif> static consteval auto getDifficultyString() {

  std::string path{};
  path.append("/");

  static constexpr auto enumerators = std::define_static_array(
      std::meta::enumerators_of(std::meta::dealias(^^Difficulty)));

  template for (constexpr auto enumerator : enumerators) {
    if constexpr (std::meta::extract<Difficulty>(enumerator) == Dif) {

      path.append(std::meta::identifier_of(enumerator));
    }
  }
  path.append("/");

  return std::define_static_string(path);
}

template <InstrumentType Type, Difficulty Dif>
  requires CustomType<Type>
Custom<Type, Dif>::Custom(std::uint32_t &noteCount, std::string filename,
                          std::string_view instrumentName,
                          InstrumentComposition<Type> instrumentComposition)
    : Base(noteCount, std::move(filename.append(getDifficultyString<Dif>())
                                    .append(instrumentName)
                                    .append(".file"))),
      m_name(instrumentName), m_composition(instrumentComposition) {}

template <Difficulty Dif>
Custom<InstrumentType::Custom_3, Dif>::Custom(
    std::uint32_t &noteCount, std::string filename,
    std::string_view instrumentName,
    InstrumentComposition<InstrumentType::Custom_3> instrumentComposition)
    : Base(noteCount, std::move(filename.append(getDifficultyString<Dif>())
                                    .append(instrumentName)
                                    .append(".file"))),
      m_name(instrumentName), m_lua(luaL_newstate()) {

  std::string file{"Instruments/"};
  file.append(std::to_string(instrumentComposition.fileNumber));
  file.append(".lua");

  if (luaL_dofile(m_lua, file.c_str()) != LUA_OK) {
    lua_close(m_lua);
    m_lua = nullptr;
  }
}

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

          this->drawNote(
              {.x = static_cast<float>(startingPositionX + i * 50),
               .y = static_cast<float>((note.timeStamp - this->m_time) * 5 +
                                       (OriginalWindowSize.y - 30))},
              Settings::getNoteTint(i), note.shape);
        }
      }
    }
  }
}

template <Difficulty Dif>
void Custom<InstrumentType::Custom_3, Dif>::draw(
    std::uint32_t startingPositionX) const noexcept {

  struct Notification {
    float position{};
    bool called{false};
    std::uint8_t colorIndex{0};
  };

  Notification notification;

  lua_pushlightuserdata(m_lua, &notification);
  lua_pushcclosure(
      m_lua,
      [](lua_State *l) -> int {
        auto *flag = static_cast<Notification *>(lua_touserdata(l, 1));

        const float position = static_cast<float>(luaL_checknumber(l, 1));
        const std::uint8_t color =
            std::saturating_cast<std::uint8_t>(luaL_optinteger(l, 2, 0));

        flag->position = position;
        flag->called = true;
        flag->colorIndex = color;

        return 0;
      },
      1);

  lua_setglobal(m_lua, "drawNote");

  for (const auto &note : this->m_activeBuffer) {

    const float posY = static_cast<float>(note.timeStamp - this->m_time) * 5 +
                       OriginalWindowSize.y - 30;

    if (m_lua) [[likely]] {
      lua_getglobal(m_lua, "Draw");

      if (!lua_isfunction(m_lua, -1)) [[unlikely]] {
      }

      lua_pushinteger(m_lua, static_cast<lua_Integer>(note.note));

      constexpr std::uint8_t argCount{1};
      constexpr std::uint8_t returnValueCount{0};

      lua_pcall(m_lua, argCount, returnValueCount, 0);

      if (notification.called) [[likely]] {

        this->drawNote(
            {.x = static_cast<float>(startingPositionX) + notification.position,
             .y = posY},
            Settings::getNoteTint(notification.colorIndex), note.shape);

      } else [[unlikely]] {

        this->drawNote(
            {.x = static_cast<float>(startingPositionX + note.note * 5),
             .y = posY},
            Settings::getNoteTint(1), note.shape);
      }

      notification.called = false;

    } else [[unlikely]] {

      this->drawNote(
          {.x = static_cast<float>(startingPositionX + note.note * 5),
           .y = posY},
          Settings::getNoteTint(1), note.shape);
    }
  }
}

template <Difficulty Dif>
bool Custom<InstrumentType::Custom_3, Dif>::getPlay(
    std::uint32_t playedNote) noexcept {

  if (m_lua) [[likely]] {
    lua_getglobal(m_lua, []<Difficulty D> consteval -> auto {
      std::string luaFunctionName{"Play"};

      luaFunctionName.append(getDifficultyString<D>());

      return std::define_static_string(luaFunctionName);
    }.template operator()<Dif>());

    if (!lua_isfunction(m_lua, -1)) [[unlikely]] {
      return Base::getPlay(playedNote);
    }

    lua_pushinteger(m_lua, static_cast<lua_Integer>(playedNote));

    constexpr std::uint8_t argCount{1};
    constexpr std::uint8_t returnValueCount{1};

    lua_pcall(m_lua, argCount, returnValueCount, 0);
    playedNote = static_cast<std::uint32_t>(lua_tointeger(m_lua, -1));
  }

  return Base::getPlay(playedNote);
}

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

} // namespace bh
