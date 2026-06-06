#pragma once

#include <fstream>
#include <thread>

namespace bh {

enum class Difficulty {
  EASY = 1,
  HARD,
};

class Instrument {
protected:
  std::thread m_loader;
  std::fstream m_file;
};

template <Difficulty Dif> struct NumberStrings {
  enum {
    Bass = 4,
    Guitar = 6,
  };
};

} // namespace bh
