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
#include <string_view>
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
    Type == InstrumentType::Bass || Type == InstrumentType::Guitar ||
    Type == InstrumentType::Custom_1;

template <InstrumentType Type>
concept CustomType =
    Type == InstrumentType::Custom_1 || Type == InstrumentType::Custom_2 ||
    Type == InstrumentType::Custom_3;

template <InstrumentType Type> struct Note {

  std::uint32_t note{};
  // Runtime progress within a chord; this field is not stored in chart files.
  std::uint32_t playedBits{};
  float timeStamp{};
  std::uint8_t shape{};
};

template <InstrumentType Type>
  requires SectionInstrumentType<Type>
struct Note<Type> {

  std::uint32_t note{};
  // Runtime progress within a chord; this field is not stored in chart files.
  std::uint32_t playedSections{};
  float timeStamp{};
  std::uint8_t shape{};
};

struct SectionLayout {
  std::uint8_t numberSections{};
  std::uint8_t bitsPerSection{};
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

template <InstrumentType Type, Difficulty Dif> class Instrument {
public:
  explicit Instrument(std::uint32_t &noteCount, std::string &&filename)
    requires CustomType<Type>;
  explicit Instrument(std::uint32_t &noteCount, std::string &&filename)
    requires(!CustomType<Type>);

  virtual ~Instrument() {
    if (IsMusicValid(m_audio)) {
      StopMusicStream(m_audio);
      UnloadMusicStream(m_audio);
    }
  };

  virtual bool getPlay(std::uint32_t playedNote) noexcept;

  virtual void draw(std::uint32_t startingPositionX) const noexcept = 0;
  void update(float dt);

  inline void loadAudio(std::string_view songPath) {
    std::string path{songPath};
    path.append([]<InstrumentType T>() consteval -> auto {
      static constexpr auto enumerators =
          std::define_static_array(std::meta::enumerators_of(^^InstrumentType));

      template for (constexpr auto enumerator : enumerators) {

        if (std::meta::extract<InstrumentType>(enumerator) == T) {

          std::string file{std::meta::identifier_of(enumerator)};
          // Enum identifiers are ASCII; std::tolower is not constexpr.
          for (auto &c : file) {
            if (c >= 'A' && c <= 'Z') {
              c += 'a' - 'A';
            }
          }
          file.append(".wav");
          return std::define_static_string(file);
        }
      }
      std::unreachable();
    }.template operator()<Type>());

    if (IsMusicValid(m_audio)) {
      StopMusicStream(m_audio);
      UnloadMusicStream(m_audio);
    }
    m_audio = LoadMusicStream(path.c_str());
    m_audio.looping = false;
  }

  inline void startAudio() noexcept {
    if (IsMusicValid(m_audio)) {
      PlayMusicStream(m_audio);
    }
  }
  template <bool Resume> inline void controlAudio() noexcept {
    if (!IsMusicValid(m_audio)) {
      return;
    }
    if constexpr (Resume) {
      ResumeMusicStream(m_audio);
    } else {
      PauseMusicStream(m_audio);
    }
  }

  inline void pause(bool p) noexcept {
    {
      std::lock_guard lock(m_bufferMutex);
      m_paused = p;
    }
    m_bufferCv.notify_all();
  }

  inline std::uint8_t getType() { return static_cast<std::uint8_t>(Type); }

protected:
  inline void drawNote(const Vector2 &position, const Color &tint,
                       std::uint8_t shape) const noexcept {

    ResourceManager::drawImage<Textures::Gameplay::Notes>(
        {.x = 150.f * (shape % 2), .y = 0, .width = 150, .height = 150},
        position, tint);
  }

  void loadFile(std::stop_token stopToken, std::string &&filename);

  void selectPlayable() noexcept;

  bool getPlay(std::uint32_t playedNote, SectionLayout layout) noexcept
    requires SectionInstrumentType<Type>;
  bool completeSelectedNote() noexcept;

  // Times are seconds relative to the instrument's unpaused playback clock.
  static constexpr double RefillAhead = 5.0;
  static constexpr double EarlyWindow = 0.75;
  static constexpr double LateWindow = 0.5;
  static constexpr std::size_t BatchSize = 256;
  static constexpr std::size_t NoNote = std::numeric_limits<std::size_t>::max();

  using NoteType = Note<Type>;
  static std::vector<NoteType> makeBatchBuffer() {
    std::vector<NoteType> buffer;
    buffer.reserve(BatchSize);
    return buffer;
  }

  std::mutex m_bufferMutex;

  Music m_audio{};

  std::uint32_t m_originalNote{};
  std::uint32_t m_playingNote{};

  // The main thread alone owns active notes. The worker fills loadingBuffer,
  // then publishes downloadingBuffer under the mutex; bufferReady prevents
  // another publication until update() has consumed the previous batch and
  // returned its storage for reuse.
  std::deque<NoteType> m_activeBuffer;
  std::vector<NoteType> m_loadingBuffer;
  std::vector<NoteType> m_downloadingBuffer;

  std::condition_variable_any m_bufferCv;
  std::uint32_t &m_noteCount;

  bool m_bufferReady{false};
  bool m_paused{true};

  std::size_t m_selectedNote{NoNote};
  double m_time{};
  // Declared last so destruction stops/joins the worker before its buffers,
  // mutex and condition variable are destroyed.
  std::jthread m_loadingThread;
};

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount,
                                  std::string &&filename)
  requires CustomType<Type>
    : m_loadingBuffer(makeBatchBuffer()),
      m_downloadingBuffer(makeBatchBuffer()), m_noteCount(noteCount),
      m_loadingThread(std::bind_front(&Instrument<Type, Dif>::loadFile, this),
                      std::move(filename)) {}

template <InstrumentType Type, Difficulty Dif>
Instrument<Type, Dif>::Instrument(std::uint32_t &noteCount,
                                  std::string &&filename)
  requires(!CustomType<Type>)
    : m_loadingBuffer(makeBatchBuffer()),
      m_downloadingBuffer(makeBatchBuffer()), m_noteCount(noteCount),
      m_loadingThread(
          std::bind_front(&Instrument<Type, Dif>::loadFile, this),
          // The caller supplies the song directory with its trailing separator.
          // Enum template arguments produce a basename such as BassHard.file.
          std::move(filename.append([this] consteval -> auto {
            constexpr auto self = std::meta::remove_cvref(^^decltype(*this));

            std::string path{};

            static constexpr auto templateArgs = [self]() consteval -> auto {
              auto templateArgs = std::meta::template_arguments_of(self);

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

  if (IsMusicValid(m_audio)) {
    UpdateMusicStream(m_audio);
  }

  if (m_activeBuffer.empty() ||
      m_activeBuffer.back().timeStamp - m_time < RefillAhead) {

    std::vector<NoteType> batch;
    {
      std::lock_guard lock(m_bufferMutex);
      if (m_bufferReady) {
        batch.swap(m_downloadingBuffer);
      }
    }
    if (!batch.empty()) {
      for (auto &note : batch) {
        note.shape = static_cast<std::uint8_t>(GetRandomValue(0, 1));
        m_activeBuffer.push_back(note);
      }
      batch.clear();
      {
        std::lock_guard lock(m_bufferMutex);
        // Keep the worker waiting until its reusable storage is back in place.
        batch.swap(m_downloadingBuffer);
        m_bufferReady = false;
      }
      m_bufferCv.notify_all();
    }
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

  double closest = std::numeric_limits<double>::infinity();

  // Charts must be ordered by timestamp: the early-window cutoff below can
  // stop the search. Choose the closest eligible chord, retaining its progress.
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
      // Records contain only uint32 note bits followed by float timestamp,
      // in the host's binary representation; never read sizeof(NoteType),
      // which also includes runtime fields and may contain padding.
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
  if constexpr (SectionInstrumentType<Type>) {
    // This virtual entry point exists for every type; sections use the
    // layout overload instead of instantiating bit-based scoring.
    return false;
  } else {
    if (m_selectedNote == NoNote || !playedNote) {
      return false;
    }
    auto &note = m_activeBuffer[m_selectedNote];
    const auto remainingBits = note.note & ~note.playedBits;
    // Reject extra or already-played bits. Valid subsets complete a chord
    // across inputs, awarding a score only after all required bits are played.
    if (playedNote & ~remainingBits) {
      return false;
    }

    note.playedBits |= playedNote;

    if (note.note & ~note.playedBits) {
      return false;
    }
    return completeSelectedNote();
  }
}

template <InstrumentType Type, Difficulty Dif>
bool Instrument<Type, Dif>::getPlay(std::uint32_t playedNote,
                                    SectionLayout layout) noexcept
  requires SectionInstrumentType<Type>
{
  if (m_selectedNote == NoNote || !playedNote) {
    return false;
  }

  auto &note = m_activeBuffer[m_selectedNote];
  const std::uint32_t sectionMask = (1u << layout.bitsPerSection) - 1u;
  std::uint32_t requiredSections{};

  for (std::uint8_t section = 0; section < layout.numberSections; ++section) {
    const auto shift = section * layout.bitsPerSection;
    const auto expected = (note.note >> shift) & sectionMask;
    if (!expected) {
      continue;
    }

    const std::uint32_t sectionBit = 1u << section;
    requiredSections |= sectionBit;
    const auto played = (playedNote >> shift) & sectionMask;
    // Easy accepts any nonzero fret on the required string. Hard compares
    // the entire section value; a wrong string never blocks another match.
    if (played && (Dif == Difficulty::Easy || played == expected)) {
      note.playedSections |= sectionBit;
    }
  }

  if (!requiredSections ||
      (note.playedSections & requiredSections) != requiredSections) {
    return false;
  }
  return completeSelectedNote();
}

template <InstrumentType Type, Difficulty Dif>
bool Instrument<Type, Dif>::completeSelectedNote() noexcept {
  m_activeBuffer.erase(m_activeBuffer.begin() + m_selectedNote);
  ++m_noteCount;
  selectPlayable();
  return true;
}

} // namespace bh
