#pragma once

#include <lua.hpp>

#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <variant>

#include "lua.h"
#include "raylib.h"

#include "config/Settings.hpp"

#include "core/Scale.hpp"

#include "gameplay/instruments/Instrument.hpp"

namespace bh {

struct CustomInstrumentComposition {
  std::string name;
  std::variant<bh::InstrumentComposition<bh::InstrumentType::Custom_1>,
               bh::InstrumentComposition<bh::InstrumentType::Custom_2>,
               bh::InstrumentComposition<bh::InstrumentType::Custom_3>>
      composition;
};

template <bh::InstrumentType Type, bh::Difficulty Dif>
  requires bh::CustomType<Type>
class Custom final : public bh::Instrument<Type, Dif> {
public:
  using Base = bh::Instrument<Type, Dif>;

  explicit Custom(std::uint32_t &noteCount, std::string filename,
                  std::string_view instrumentName,
                  bh::InstrumentComposition<Type> instrumentComposition);
  ~Custom() override = default;

  bool getPlay(std::uint32_t playedNote) noexcept override {
    return Base::getPlay(playedNote);
  }

  void draw(std::uint32_t startingPositionX) const noexcept override;

  inline auto getComposition() const noexcept { return m_composition; }
  inline std::string getName() const noexcept { return m_name; }

private:
  std::string m_name;
  bh::InstrumentComposition<Type> m_composition;
};

template <bh::Difficulty Dif>
class Custom<bh::InstrumentType::Custom_3, Dif> final
    : public bh::Instrument<bh::InstrumentType::Custom_3, Dif> {
public:
  using Base = bh::Instrument<bh::InstrumentType::Custom_3, Dif>;

  explicit Custom(
      std::uint32_t &noteCount, std::string filename,
      std::string_view instrumentName,
      bh::InstrumentComposition<bh::InstrumentType::Custom_3> instrumentComposition);

  ~Custom() override {
    if (m_lua) {
      ::lua_close(m_lua);
    }
  };

  bool getPlay(std::uint32_t playedNote) noexcept override;

  void draw(std::uint32_t startingPositionX) const noexcept override;

  inline auto getComposition() const noexcept { return m_composition; }
  inline std::string getName() const noexcept { return m_name; }

private:
  std::string m_name;
  ::lua_State *m_lua;

  // Lua's drawNote callback records a request rather than drawing directly.
  // C++ adds the player's X origin, timestamp-based Y, tint and sprite shape.
  struct Notification {
    float position{};
    bool called{false};
    std::uint8_t colorIndex{0};
  };

  mutable Notification m_notification;
  bh::InstrumentComposition<bh::InstrumentType::Custom_3> m_composition;
};

template <bh::Difficulty Dif> static consteval auto getDifficultyString() {

  std::string path{};

  static constexpr auto enumerators = std::define_static_array(
      std::meta::enumerators_of(std::meta::dealias(^^bh::Difficulty)));

  template for (constexpr auto enumerator : enumerators) {
    if constexpr (std::meta::extract<bh::Difficulty>(enumerator) == Dif) {

      path.append(std::meta::identifier_of(enumerator));
    }
  }

  return std::define_static_string(path);
}

template <bh::InstrumentType Type, bh::Difficulty Dif>
  requires bh::CustomType<Type>
Custom<Type, Dif>::Custom(std::uint32_t &noteCount, std::string filename,
                          std::string_view instrumentName,
                          bh::InstrumentComposition<Type> instrumentComposition)
    : Base(noteCount, std::move(filename.append(instrumentName)
                                    .append(getDifficultyString<Dif>())
                                    .append(".file"))),
      m_name(instrumentName), m_composition(instrumentComposition) {}

template <bh::Difficulty Dif>
Custom<bh::InstrumentType::Custom_3, Dif>::Custom(
    std::uint32_t &noteCount, std::string filename,
    std::string_view instrumentName,
    bh::InstrumentComposition<bh::InstrumentType::Custom_3> instrumentComposition)
    : Base(noteCount, std::move(filename.append(instrumentName)
                                    .append(getDifficultyString<Dif>())
                                    .append(".file"))),
      m_name(instrumentName), m_lua(::luaL_newstate()),
      m_composition(instrumentComposition) {

  if (!m_lua) {
    return;
  }

  std::string file{"Instruments/"};
  file.append(std::to_string(instrumentComposition.fileNumber));
  file.append(".lua");

  if (luaL_dofile(m_lua, file.c_str()) != LUA_OK) {
    ::lua_close(m_lua);
    m_lua = nullptr;
  }
}

template <bh::InstrumentType Type, bh::Difficulty Dif>
  requires bh::CustomType<Type>
void Custom<Type, Dif>::draw(std::uint32_t startingPositionX) const noexcept {
  if constexpr (Type == bh::InstrumentType::Custom_1) {

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
                                       (bh::OriginalWindowSize.y - 30))},
              bh::Settings::getNoteTint(i), note.shape);
        }
      }
    }

  } else if constexpr (Type == bh::InstrumentType::Custom_2) {

    for (const auto &note : this->m_activeBuffer) {
      if (note.timeStamp - this->m_time > 2.f) {
        break;
      }
      const auto bits =
          (Dif == bh::Difficulty::Easy) * m_composition.NumberEffectiveBitsEasy +
          (Dif == bh::Difficulty::Hard) * m_composition.NumberEffectiveBitsHard;

      for (std::uint8_t i{}; i < std::min<unsigned>(bits, 30); ++i) {

        if (bool playedBit = (note.note >> i) & 1; playedBit) [[unlikely]] {

          this->drawNote(
              {.x = static_cast<float>(startingPositionX + i * 50),
               .y = static_cast<float>((note.timeStamp - this->m_time) * 5 +
                                       (bh::OriginalWindowSize.y - 30))},
              bh::Settings::getNoteTint(i), note.shape);
        }
      }
    }
  }
}

template <bh::Difficulty Dif>
void Custom<bh::InstrumentType::Custom_3, Dif>::draw(
    std::uint32_t startingPositionX) const noexcept {

  m_notification = {};

  const bool existsLua = m_lua;

  if (existsLua) [[likely]] {
    ::lua_pushlightuserdata(m_lua, &m_notification);

    ::lua_pushcclosure(
        m_lua,
        [](::lua_State *l) -> int {
          auto *flag = static_cast<Notification *>(
              ::lua_touserdata(l, lua_upvalueindex(1)));

          const float position = static_cast<float>(::luaL_checknumber(l, 1));
          const std::uint8_t color =
              std::saturating_cast<std::uint8_t>(::luaL_optinteger(l, 2, 0));

          flag->position = position;
          flag->called = true;
          flag->colorIndex = color;

          return 0;
        },
        1);

    ::lua_setglobal(m_lua, "drawNote");
  }

  for (const auto &note : this->m_activeBuffer) {

    const float posY = static_cast<float>(note.timeStamp - this->m_time) * 5 +
                       bh::OriginalWindowSize.y - 30;

    if (existsLua) [[likely]] {
      ::lua_getglobal(m_lua, "Draw");

      if (lua_isfunction(m_lua, -1)) [[likely]] {
        ::lua_pushinteger(m_lua, static_cast<::lua_Integer>(note.note));

        if (lua_pcall(m_lua, 1, 0, 0) != LUA_OK) [[unlikely]] {
          lua_pop(m_lua, 1);
          m_notification.called = false;
        }
      } else [[unlikely]] {
        lua_pop(m_lua, 1);
      }

      if (m_notification.called) [[likely]] {

        this->drawNote({.x = static_cast<float>(startingPositionX) +
                             m_notification.position,
                        .y = posY},
                       bh::Settings::getNoteTint(m_notification.colorIndex),
                       note.shape);

      } else [[unlikely]] {

        this->drawNote(
            {.x = static_cast<float>(startingPositionX + note.note * 5),
             .y = posY},
            bh::Settings::getNoteTint(1), note.shape);
      }

      m_notification.called = false;

    } else [[unlikely]] {

      this->drawNote(
          {.x = static_cast<float>(startingPositionX + note.note * 5),
           .y = posY},
          bh::Settings::getNoteTint(1), note.shape);
    }
  }
}

template <bh::Difficulty Dif>
bool Custom<bh::InstrumentType::Custom_3, Dif>::getPlay(
    std::uint32_t playedNote) noexcept {
  // PlayEasy/PlayHard transforms controller bits; the base class still owns
  // timing and chord completion. A missing function uses the original input.

  if (m_lua) [[likely]] {
    ::lua_getglobal(m_lua, []<bh::Difficulty D> consteval -> auto {
      std::string luaFunctionName{"Play"};

      luaFunctionName.append(getDifficultyString<D>());

      return std::define_static_string(luaFunctionName);
    }.template operator()<Dif>());

    if (!lua_isfunction(m_lua, -1)) [[unlikely]] {
      lua_pop(m_lua, 1);
      return Base::getPlay(playedNote);
    }

    ::lua_pushinteger(m_lua, static_cast<::lua_Integer>(playedNote));

    if (lua_pcall(m_lua, 1, 1, 0) == LUA_OK) [[likely]] {
      if (::lua_isinteger(m_lua, -1)) [[likely]] {
        playedNote = static_cast<std::uint32_t>(lua_tointeger(m_lua, -1));
      }
    }
    lua_pop(m_lua, 1);
  }

  return Base::getPlay(playedNote);
}

template <>
bool Custom<bh::InstrumentType::Custom_1, bh::Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<bh::InstrumentType::Custom_1, bh::Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<bh::InstrumentType::Custom_2, bh::Difficulty::Easy>::getPlay(
    std::uint32_t playedNote) noexcept;

template <>
bool Custom<bh::InstrumentType::Custom_2, bh::Difficulty::Hard>::getPlay(
    std::uint32_t playedNote) noexcept;

} // namespace bh
