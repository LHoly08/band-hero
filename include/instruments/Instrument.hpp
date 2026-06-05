#pragma once

namespace bh {

enum class Difficulty {
  EASY = 1,
  HARD,
};

class Instrument {};

template <Difficulty Dif> struct NumberStrings {
  enum {
    Bass = 4,
    Guitar = 6,
  };
};

} // namespace bh
