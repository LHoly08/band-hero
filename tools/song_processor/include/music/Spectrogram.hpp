#pragma once

#include "Tensor.hpp"
#include "Audio.hpp"

inline constexpr std::size_t STFTFFTSize = 2048;
inline constexpr std::size_t STFTHopSize = 512;

Tensor stft(const Audio& audio);
