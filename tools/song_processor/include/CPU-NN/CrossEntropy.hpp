#pragma once

#include "Tensor.hpp"
#include <span>

float crossEntropy(
    const Tensor &probabilities,
    std::span<const std::size_t> labels);

Tensor crossEntropyBackward(const Tensor &probabilities, std::span<const std::size_t> labels);