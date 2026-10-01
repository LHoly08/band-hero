#pragma once

#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <limits>
#include <meta>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "raylib.h"

#include "core/ResourceManager.hpp"

namespace bh {

enum class Difficulty : std::uint8_t {
  Easy = 0,
  Hard,
};

enum class InstrumentType : std::uint8_t {
  Bass = 0,
  Drums,
  Guitar,
  Custom_1,
  Custom_2,
  Custom_3,
};

template <InstrumentType Type>
concept SectionInstrumentType =
    Type != InstrumentType::Drums && Type != InstrumentType::Custom_2;

template <InstrumentType Type>
concept CustomType =
    Type == InstrumentType::Custom_1 || Type == InstrumentType::Custom_2 ||
    Type == InstrumentType::Custom_3;

template <InstrumentType Type> struct Note {

  std::uint32_t note{};
  std::uint32_t playedBits{};
  float timeStamp{};
  std::uint8_t shape{};
};

template <InstrumentType Type>
  requires SectionInstrumentType<Type>
struct Note<Type> {

  std::uint32_t note{};
  std::uint32_t playedBits{};
  float timeStamp{};
  std::uint8_t shape{};
};

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

template <> struct InstrumentComposition<InstrumentType::Custom_3> {
  std::uint16_t fileNumber{};
};

// Size: ? | Align: 8
template <InstrumentType Type, Difficulty Dif> class Instrument {
public:
  explicit Instrument(std::uint32_t &noteCount, std::string &&filename)
    requires CustomType<Type>;
  explicit Instrument(std::uint32_t &noteCount, std::string &&filename)
    requires(!CustomType<Type>);
  virtual ~Instrument() = default;

  virtual bool getPlay(std::uint32_t playedNote) noexcept;

  virtual void draw(std::uint32_t startingPositionX) const noexcept = 0;
  void update(float dt);

  inline void pause(bool p) noexcept {
    {
      std::lock_guard lock(m_bufferMutex);
      m_paused = p;
    }
    m_bufferCv.notify_all();
  }

protected:
  inline void drawNote(const Vector2 &position, const Color &tint,
                       std::uint8_t shape) const noexcept {

    ResourceManager::drawImage<Textures::Gameplay::Notes>(
        {.x = 150.f * (shape % 2), .y = 0, .width = 150, .height = 150},
        position, tint);
  }

  void loadFile(std::stop_token stopToken, std::string &&filename);

  void selectPlayable() noexcept;

  static constexpr double RefillAhead = 5.0;
  static constexpr double EarlyWindow = 0.75;
  static constexpr double LateWindow = 0.5;
  static constexpr std::size_t BatchSize = 256;
  static constexpr std::size_t NoNote = std::numeric_limits<std::size_t>::max();

  using NoteType = Note<Type>;
  std::mutex m_bufferMutex;

  std::uint32_t m_originalNote{};
  std::uint32_t m_playingNote{};

  std::deque<NoteType> m_activeBuffer;
  std::vector<NoteType> m_loadingBuffer;
  std::vector<NoteType> m_downloadingBuffer;

  std::condition_variable_any m_bufferCv;
  std::uint32_t &m_noteCount;

  bool m_bufferReady{false};
  bool m_paused{true};

  std::size_t m_selectedNote{NoNote};
  double m_time{};
  std::jthread m_loadingThread;
};

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount,
                                  std::string &&filename)
  requires CustomType<Type>
    : m_noteCount(noteCount),
      m_loadingThread(std::bind_front(&Instrument<Type, Dif>::loadFile, this),
                      std::move(filename)) {}

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount,
                                  std::string &&filename)
  requires(!CustomType<Type>)
    : m_noteCount(noteCount),
      m_loadingThread(
          std::bind_front(&Instrument<Type, Dif>::loadFile, this),
          std::move(filename.append([this] consteval -> auto {
            constexpr auto self = std::meta::remove_cvref(^^decltype(*this));

            std::string path{};

            static constexpr auto templateArgs = [self]() consteval -> auto {
              auto templateArgs = std::meta::template_arguments_of(self);

              std::reverse(templateArgs.begin(), templateArgs.end());

              return std::define_static_array(templateArgs);
            }();

            template for (constexpr auto arg : templateArgs) {
              using T = [:std::meta::type_of(arg):];

              if constexpr (std::meta::is_enum_type(std::meta::dealias(^^T))) {

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

            path.append(".file");

            return std::define_static_string(path);
          }()))) {}

template <InstrumentType Type, Difficulty Dif>
void Instrument<Type, Dif>::update(float dt) {
  {
    std::lock_guard lock(m_bufferMutex);
    if (m_paused) {
      return;
    }
  }
  m_time += dt;

  if (m_activeBuffer.empty() ||
      m_activeBuffer.back().timeStamp - m_time < RefillAhead) {
    {
      std::lock_guard lock(m_bufferMutex);
      if (m_bufferReady) {
        for (auto &note : m_downloadingBuffer) {
          note.shape = static_cast<std::uint8_t>(GetRandomValue(0, 1));
          m_activeBuffer.push_back(note);
        }
        m_downloadingBuffer.clear();
        m_bufferReady = false;
      }
    }
    m_bufferCv.notify_all();
  }

  while (!m_activeBuffer.empty() &&
         m_time - m_activeBuffer.front().timeStamp >= LateWindow) {
    m_activeBuffer.pop_front();
    ++m_noteCount;
  }
  selectPlayable();
}

template <InstrumentType Type, Difficulty Dif>
void Instrument<Type, Dif>::selectPlayable() noexcept {
  m_selectedNote = NoNote;
  m_originalNote = m_playingNote = 0;

  double closest = std::numeric_limits<double>::infinity();

  for (std::size_t i = 0; i < m_activeBuffer.size(); ++i) {

    const auto &note = m_activeBuffer[i];
    const double offset = note.timeStamp - m_time;

    if (offset >= EarlyWindow) {
      break;
    }
    if (offset <= -LateWindow || note.note == 0) {
      continue;
    }
    if (std::abs(offset) < closest) {
      closest = std::abs(offset);
      m_selectedNote = i;
      m_originalNote = note.note;
      m_playingNote = note.note & ~note.playedBits;
    }
  }
}

template <InstrumentType Type, Difficulty Dif>
void Instrument<Type, Dif>::loadFile(std::stop_token stopToken,
                                     std::string &&filename) {
  if (filename.empty()) {
    return;
  }

  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    TraceLog(LOG_WARNING, "Could not open chart: %s", filename.c_str());
    return;
  }

  bool finished = false;

  while (!stopToken.stop_requested() && !finished) {
    {
      std::unique_lock lock(m_bufferMutex);
      if (!m_bufferCv.wait(lock, stopToken,
                           [this] { return !m_paused && !m_bufferReady; })) {
        return;
      }
    }
    m_loadingBuffer.clear();

    for (std::size_t i = 0; i < BatchSize && !stopToken.stop_requested(); ++i) {
      NoteType note{};
      if (!file.read(reinterpret_cast<char *>(&note.note), sizeof(note.note)) ||
          !file.read(reinterpret_cast<char *>(&note.timeStamp),
                     sizeof(note.timeStamp))) {
        finished = true;
        break;
      }

      m_loadingBuffer.push_back(note);
    }
    if (stopToken.stop_requested()) {
      return;
    }
    if (!m_loadingBuffer.empty()) {
      std::lock_guard lock(m_bufferMutex);
      m_loadingBuffer.swap(m_downloadingBuffer);
      m_bufferReady = true;
    }
  }
}

template <InstrumentType Type, Difficulty Dif>
bool Instrument<Type, Dif>::getPlay(std::uint32_t playedNote) noexcept {
  if (m_selectedNote == NoNote || !playedNote ||
      (playedNote & ~m_playingNote) || !m_playingNote) {
    return false;
  }

  auto &note = m_activeBuffer[m_selectedNote];
  note.playedBits |= playedNote;
  m_playingNote = m_originalNote & ~note.playedBits;

  if (m_playingNote) {
    return false;
  }

  m_activeBuffer.erase(m_activeBuffer.begin() + m_selectedNote);
  ++m_noteCount;
  selectPlayable();
  return true;
}

} // namespace bh
