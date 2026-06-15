#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <semaphore>
#include <thread>
#include <vector>

namespace bh {

enum class Difficulty {
  EASY = 1,
  HARD,
};

class Note {
public:
  Note(sf::Vector2f position, std::uint32_t note) noexcept;
  ~Note() = default;

  void update(float dt) noexcept;
  void draw(sf::RenderTarget &target) const noexcept;

  inline std::uint32_t getNote() const noexcept { return m_note; }

private:
  sf::RenderTexture m_texture;
  sf::Vector2f m_position;

  const std::uint32_t m_note;
};

template <Difficulty Dif> class Instrument {
public:
  Instrument(std::string filename) noexcept;
  virtual ~Instrument() = default;
  Instrument(const Instrument &) = delete;
  void operator=(const Instrument &) = delete;
  Instrument(Instrument &&) = delete;
  Instrument &operator=(Instrument &&) = delete;

  inline bool getPlay(const std::uint32_t &played) const noexcept {
    return !(played ^ m_toPlay);
  }
  void loadFile(std::stop_token stop_token, std::string filename) noexcept;
  void startThread() noexcept;
  void pauseThread() noexcept;

protected:
  std::vector<Note> m_activeByffer;
  std::vector<Note> m_buffer1;
  std::vector<Note> m_buffer2;

  std::jthread m_loader;
  std::mutex m_mtx;

  std::condition_variable m_pausedCondition{};
  std::binary_semaphore m_signalToProcess{0};
  std::binary_semaphore m_signalToGenerate{1};

  std::uint32_t m_toPlay;
  float m_yCord{};
  bool m_paused{false};
};

template <Difficulty Dif>
void Instrument<Dif>::loadFile(std::stop_token stop_token,
                               std::string filename) noexcept {
  while (!stop_token.stop_requested()) {

    {
      std::unique_lock lk(m_mtx);
      m_pausedCondition.wait(lk, [&] { return !m_paused; });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

template <Difficulty Dif> void Instrument<Dif>::startThread() noexcept {
  {
    std::lock_guard lk(m_mtx);
    m_paused = false;
  }
  m_pausedCondition.notify_one();
}

template <Difficulty Dif> void Instrument<Dif>::pauseThread() noexcept {
  {
    std::lock_guard lk(m_mtx);
    m_paused = true;
  }
  m_pausedCondition.notify_one();
}

template <Difficulty Dif> struct NumberStrings;

template <> struct NumberStrings<Difficulty::EASY> {
  enum {
    Bass = 4,
    Guitar = 6,
  };
};

template <> struct NumberStrings<Difficulty::HARD> {
  enum {
    Bass = 4,
    Guitar = 6,
  };
};

} // namespace bh
