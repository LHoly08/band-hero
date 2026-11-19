#pragma once

#include "TensorCuda.cuh"
#include <span>

float crossEntropyCuda(const TensorCuda& probabilities, std::span<const std::size_t> labels);

TensorCuda crossEntropyBackwardCuda(const TensorCuda &probabilities,
                                    std::span<const std::size_t> labels);
