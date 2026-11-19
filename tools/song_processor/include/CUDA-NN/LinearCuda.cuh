#pragma once

#include "Tensor.hpp"
#include "TensorCuda.cuh"

#include <cstddef>
#include <istream>
#include <ostream>

class LinearCuda {
public:
  LinearCuda(std::size_t inputSize, std::size_t outputSize);

  LinearCuda(const Tensor &weights, const Tensor &bias);

  [[nodiscard]]
  TensorCuda forward(const TensorCuda &input);

  [[nodiscard]]
  TensorCuda backward(const TensorCuda &gradient, float learningRate);

  void save(std::ostream &out) const;

  [[nodiscard]]
  static LinearCuda load(std::istream &in);

private:
  TensorCuda m_weights;
  TensorCuda m_bias;

  const TensorCuda *m_input{nullptr};
};