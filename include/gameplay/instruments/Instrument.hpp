#pragma once

#include <atomic>
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
    if (IsMusicValid(m_audio)) {
      SetMusicVolume(m_audio, m_audioVolume);
    }
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

  inline std::uint32_t getLoadedNoteCount() const noexcept {
    return m_loadedNotes.load(std::memory_order_relaxed);
  }

  inline std::uint32_t getHitNoteCount() const noexcept { return m_hitNotes; }
  inline std::uint32_t getBestStreak() const noexcept { return m_bestStreak; }
  inline bool hadIncorrectInput() const noexcept { return m_incorrectInput; }
  inline bool hasCompletedChart() const noexcept {
    const auto loaded = getLoadedNoteCount();
    return m_chartLoaded.load(std::memory_order_relaxed) && loaded > 0 &&
           m_noteCount == loaded;
  }

  // Freeze results before the end menu reads counters or awards medals.
  inline void finishSong() noexcept {
    pause(true);
    m_loadingThread.request_stop();
    if (m_loadingThread.joinable()) {
      m_loadingThread.join();
    }
    controlAudio<false>();
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

  bool getPlay(std::uint32_t playedNote, SectionLayout layout) noexcept
    requires SectionInstrumentType<Type>;
  bool completeSelectedNote() noexcept;
  void setAudioVolume(float volume) noexcept {
    if (m_audioVolume == volume) {
      return;
    }
    m_audioVolume = volume;
    if (IsMusicValid(m_audio)) {
      SetMusicVolume(m_audio, volume);
    }
  }
  bool rejectInput() noexcept {
    m_incorrectInput = true;
    m_streak = 0;
    return false;
  }

  // Times are seconds relative to the instrument's unpaused playback clock.
  static constexpr double RefillAhead = 5.0;
  static constexpr double EarlyWindow = 0.75;
  static constexpr double LateWindow = 0.5;
  static constexpr std::size_t BatchSize = 256;
  static constexpr std::size_t NoNote = std::numeric_limits<std::size_t>::max();
  static constexpr float MissedNoteVolume = .25f;

  using NoteType = Note<Type>;
  static std::vector<NoteType> makeBatchBuffer() {
    std::vector<NoteType> buffer;
    buffer.reserve(BatchSize);
    return buffer;
  }

  std::mutex m_bufferMutex;

  Music m_audio{};
  float m_audioVolume{1.f};

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
  // Count complete chart records (one per chord), including prefetched notes.
  // The loader writes this counter while the main thread reads it.
  std::atomic<std::uint32_t> m_loadedNotes{};
  std::atomic<bool> m_chartLoaded{};
  std::uint32_t m_hitNotes{};
  std::uint32_t m_streak{};
  std::uint32_t m_bestStreak{};
  bool m_incorrectInput{};

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
    m_streak = 0;
    setAudioVolume(MissedNoteVolume);
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
      if (!file.read(reinterpret_cast<char *>(&note.note), sizeof(note.note))) {
        // A clean EOF completes the chart; a truncated record does not.
        m_chartLoaded.store(file.eof() && file.gcount() == 0,
                            std::memory_order_relaxed);
        finished = true;
        break;
      }
      if (!file.read(reinterpret_cast<char *>(&note.timeStamp),
                     sizeof(note.timeStamp))) {
        finished = true;
        break;
      }
      m_loadingBuffer.push_back(note);
      m_loadedNotes.fetch_add(1, std::memory_order_relaxed);
    }
    // Detect EOF even when the final batch contains exactly BatchSize records.
    if (!finished && !stopToken.stop_requested() &&
        file.peek() == std::ifstream::traits_type::eof()) {
      m_chartLoaded.store(file.eof(), std::memory_order_relaxed);
      finished = true;
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
    if (!playedNote) {
      return false;
    }
    if (m_selectedNote == NoNote) {
      return rejectInput();
    }
    auto &note = m_activeBuffer[m_selectedNote];
    const auto remainingBits = note.note & ~note.playedBits;
    // Reject extra or already-played bits. Valid subsets complete a chord
    // across inputs, awarding a score only after all required bits are played.
    if (playedNote & ~remainingBits) {
      return rejectInput();
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
  if (!playedNote) {
    return false;
  }
  if (m_selectedNote == NoNote) {
    return rejectInput();
  }

  auto &note = m_activeBuffer[m_selectedNote];
  const std::uint32_t sectionMask = (1u << layout.bitsPerSection) - 1u;
  std::uint32_t requiredSections{};
  std::uint32_t matchingSections{};

  for (std::uint8_t section = 0; section < layout.numberSections; ++section) {
    const auto shift = section * layout.bitsPerSection;
    const auto expected = (note.note >> shift) & sectionMask;
    const auto played = (playedNote >> shift) & sectionMask;
    const std::uint32_t sectionBit = 1u << section;
    if (played && (!expected || (note.playedSections & sectionBit) ||
                   (Dif == Difficulty::Hard && played != expected))) {
      return rejectInput();
    }
    if (!expected) {
      continue;
    }

    requiredSections |= sectionBit;
    // Easy accepts any nonzero fret on the required string. Hard compares
    // the entire section value; a wrong string never blocks another match.
    if (played && (Dif == Difficulty::Easy || played == expected)) {
      matchingSections |= sectionBit;
    }
  }

  note.playedSections |= matchingSections;
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
  ++m_hitNotes;
  setAudioVolume(1.f);
  ++m_streak;
  if (m_streak > m_bestStreak) {
    m_bestStreak = m_streak;
  }
  selectPlayable();
  return true;
}

} // namespace bh
