#pragma once

#include "TensorCuda.cuh"

[[nodiscard]]
TensorCuda softmaxCuda(const TensorCuda &input);