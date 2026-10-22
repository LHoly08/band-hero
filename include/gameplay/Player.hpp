#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

#include "raylib.h"

#include "gameplay/instruments/Bass.hpp"
#include "gameplay/instruments/Custom.hpp"
#include "gameplay/instruments/Drums.hpp"
#include "gameplay/instruments/Guitar.hpp"
#include "gameplay/instruments/Instrument.hpp"

#include "medals/AchievementsManager.hpp"
#include "serial/ControllerInput.hpp"

namespace bh {

class PlayerBase {
public:
  virtual ~PlayerBase() = default;

  virtual void draw() const noexcept = 0;
  virtual void update(float dt) = 0;

  virtual void play(std::uint32_t notePlayed) noexcept = 0;
  virtual void resetControllerState() noexcept = 0;
  virtual void pauseInstrument(bool p) noexcept = 0;
  virtual void loadAudio(std::string_view songPath) = 0;
  virtual void startAudio() noexcept = 0;
  virtual void controlAudio(bool resume) noexcept = 0;
  virtual std::uint8_t getInstrumentType() noexcept = 0;
  virtual std::unique_ptr<PlayerBase> changeSong(std::string songPath) = 0;
  virtual std::vector<bh::Achievements> getUnlockedAchievements() = 0;
  virtual float getCompletion() = 0;
  virtual std::uint32_t getLoadedNoteCount() const noexcept = 0;
  virtual std::uint32_t getHitNoteCount() const noexcept = 0;
  virtual bool hasCompletedSong() const noexcept = 0;
  virtual void finishSong() noexcept = 0;

  inline static void setPlayerCount(std::uint8_t count) noexcept {
    assert(count != 0 && count <= 4);
    PlayerCount = count;
  }

protected:
  inline static std::uint8_t PlayerCount = 1;
};

template <bh::InstrumentType Type, bh::Difficulty Dif> struct InstrumentFor;

template <bh::Difficulty Dif> struct InstrumentFor<bh::InstrumentType::Bass, Dif> {
  using type = bh::Bass<Dif>;
};

template <bh::Difficulty Dif> struct InstrumentFor<bh::InstrumentType::Guitar, Dif> {
  using type = bh::Guitar<Dif>;
};

template <bh::Difficulty Dif> struct InstrumentFor<bh::InstrumentType::Drums, Dif> {
  using type = bh::Drums<Dif>;
};

template <bh::InstrumentType Type, bh::Difficulty Dif>
  requires bh::CustomType<Type>
struct InstrumentFor<Type, Dif> {
  using type = bh::Custom<Type, Dif>;
};

template <bh::InstrumentType Type, bh::Difficulty Dif>
using InstrumentFor_t = typename InstrumentFor<Type, Dif>::type;

template <bh::InstrumentType Type, bh::Difficulty Dif>
class Player final : public PlayerBase {
public:
  template <typename... Args> Player(std::uint32_t id, Args &&...args);
  ~Player() override = default;
  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;
  Player(Player &&) = delete;
  Player &operator=(Player &&) = delete;

  inline void pauseInstrument(bool p) noexcept override {
    m_instrument.pause(p);
  }

  inline void play(std::uint32_t notePlayed) noexcept override {
    std::uint32_t presses{};
    if constexpr (Type == bh::InstrumentType::Bass) {
      presses = m_controllerState.update(notePlayed, bh::BassComposition<Dif>::Strings,
                                         bh::BassComposition<Dif>::FretBits);
    } else if constexpr (Type == bh::InstrumentType::Guitar) {
      presses = m_controllerState.update(notePlayed, bh::GuitarComposition<Dif>::Strings,
                                         bh::GuitarComposition<Dif>::FretBits);
    } else if constexpr (Type == bh::InstrumentType::Custom_1) {
      const auto composition = m_instrument.getComposition();
      presses = m_controllerState.update(notePlayed, composition.NumberSections,
                                         composition.NumberBitsSection);
    } else {
      presses = m_controllerState.update(notePlayed);
    }
    if (presses) {
      m_instrument.getPlay(presses);
    }
  }
  inline void resetControllerState() noexcept override {
    m_controllerState = {};
  }

  inline void draw() const noexcept override {
    m_instrument.draw((bh::OriginalWindowSize.x / PlayerCount) * id);
  }

  inline void update(float dt) override { m_instrument.update(dt); }

  inline void loadAudio(std::string_view songPath) override {
    m_instrument.loadAudio(songPath);
  }
  inline void startAudio() noexcept override { m_instrument.startAudio(); }
  inline void controlAudio(bool resume) noexcept override {
    if (resume) {
      m_instrument.template controlAudio<true>();
    } else {
      m_instrument.template controlAudio<false>();
    }
  }
  inline std::uint8_t getInstrumentType() noexcept override {
    return m_instrument.getType();
  }
  inline std::unique_ptr<PlayerBase> changeSong(std::string songPath) override {
    if constexpr (bh::CustomType<Type>) {
      return std::make_unique<Player<Type, Dif>>(id, std::move(songPath),
                                                 m_instrument.getName(),
                                                 m_instrument.getComposition());
    } else {
      return std::make_unique<Player<Type, Dif>>(id, std::move(songPath));
    }
  }
  std::vector<bh::Achievements> getUnlockedAchievements() override;
  inline std::uint32_t getLoadedNoteCount() const noexcept override {
    return m_instrument.getLoadedNoteCount();
  }
  inline std::uint32_t getHitNoteCount() const noexcept override {
    return m_instrument.getHitNoteCount();
  }
  inline bool hasCompletedSong() const noexcept override {
    return m_instrument.hasCompletedChart();
  }
  inline void finishSong() noexcept override { m_instrument.finishSong(); }
  inline float getCompletion() override {
    const auto loaded = getLoadedNoteCount();
    return loaded == 0 ? 0.f : static_cast<float>(getHitNoteCount()) / loaded;
  }

private:
  const std::uint32_t id;

  std::uint32_t m_passedNotes{};
  bh::ControllerState m_controllerState;
  InstrumentFor_t<Type, Dif> m_instrument;
};

template <bh::InstrumentType Type, bh::Difficulty Dif>
template <typename... Args>
Player<Type, Dif>::Player(std::uint32_t id, Args &&...args)
    : id(id), m_instrument(m_passedNotes, std::forward<Args>(args)...) {}

template <bh::InstrumentType Type, bh::Difficulty Dif>
std::vector<bh::Achievements> Player<Type, Dif>::getUnlockedAchievements() {
  std::vector<bh::Achievements> earned;
  if (!hasCompletedSong()) {
    return earned;
  }
  earned.push_back(bh::Achievements::FirstGig);
  earned.push_back(Dif == bh::Difficulty::Hard ? bh::Achievements::Hard
                                          : bh::Achievements::Easy);

  const bool allHit = getLoadedNoteCount() == getHitNoteCount();
  const bool full = allHit && !m_instrument.hadIncorrectInput();

  if (allHit) {
    earned.push_back(bh::Achievements::Completion);
  }

  if (full) {
    earned.push_back(bh::Achievements::FullCombo);
  }
  if (m_instrument.getBestStreak() >= bh::AchievementsManager::StreakTarget) {
    earned.push_back(bh::Achievements::Streak);
  }

  constexpr std::array<bh::Achievements, 3> InstrumentAchievements{
      bh::Achievements::Bass, bh::Achievements::Drums, bh::Achievements::Guitar};

  constexpr auto index = static_cast<std::uint8_t>(Type);

  constexpr bool isInstrument = index < InstrumentAchievements.size();

  if constexpr (isInstrument) {
    earned.push_back(InstrumentAchievements[index]);
  }
  if constexpr (isInstrument && Dif == bh::Difficulty::Hard) {
    if (full) {
      constexpr std::array<bh::Achievements, 3> InstrumentMastery{
          bh::Achievements::BassMastery, bh::Achievements::DrumsMastery,
          bh::Achievements::GuitarMastery};
      earned.push_back(InstrumentMastery[index]);
    }
  }

  return earned;
}

} // namespace bh
