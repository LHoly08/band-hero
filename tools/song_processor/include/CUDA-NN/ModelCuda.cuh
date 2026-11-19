#pragma once

#include "LinearCuda.cuh"
#include "ReLUCuda.cuh"
#include "TensorCuda.cuh"

#include <filesystem>
#include <optional>

class ModelCuda {
public:
  ModelCuda();

  [[nodiscard]]
  TensorCuda forward(const TensorCuda &input);

  [[nodiscard]]
  TensorCuda backward(const TensorCuda &gradient, float learningRate);

  void save(const std::filesystem::path &path) const;

  [[nodiscard]]
  static ModelCuda load(const std::filesystem::path &path);

private:
  ModelCuda(LinearCuda &&l1, LinearCuda &&l2);

  LinearCuda m_l1;
  ReLUCuda m_relu;
  LinearCuda m_l2;

  // Saved forward activations needed by backward.
  std::optional<TensorCuda> m_h1;
  std::optional<TensorCuda> m_h2;
};