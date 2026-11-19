#pragma once

#include "Tensor.hpp"
#include "TensorCuda.cuh"

[[nodiscard]]
Tensor matmulCuda(const Tensor &a, const Tensor &b);

[[nodiscard]]
TensorCuda matmulCuda(const TensorCuda &a, const TensorCuda &b);

[[nodiscard]]
TensorCuda matmulTiled(const TensorCuda &a, const TensorCuda &b);
