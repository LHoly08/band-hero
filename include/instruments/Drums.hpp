#pragma once

#include <bitset>

#include "instruments/Instrument.hpp"

namespace bh {

template <unsigned char NumberPedals, unsigned char NumberDrums>
class Drums : public Instrument {
public:
  bool getPlay(const std::bitset<NumberPedals> &pedals,
               const std::bitset<NumberDrums> &drums);

private:
  std::bitset<NumberPedals> m_pedals;
  std::bitset<NumberDrums> m_drums;
};

} // namespace bh
