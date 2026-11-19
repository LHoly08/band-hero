#pragma once

#include "Tensor.hpp"

std::vector<float> spectralFlux(const Tensor& spectrogram, bool useLogCompression = true);