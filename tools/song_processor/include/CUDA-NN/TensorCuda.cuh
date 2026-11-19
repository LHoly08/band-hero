#pragma once

#include "CudaCheck.cuh"
#include "Tensor.hpp"

#include <cstddef>
#include <utility>

class TensorCuda {
public:
  TensorCuda(std::size_t rows, std::size_t cols) : m_rows(rows), m_cols(cols) {
    if (rows && cols) {
      CUDA_CHECK(cudaMalloc(&m_data, sizeBytes()));
    }
  }

  TensorCuda(const Tensor &tensor) : TensorCuda(tensor.rows(), tensor.cols()) {
    CUDA_CHECK(
        cudaMemcpy(m_data, tensor.data(), sizeBytes(), cudaMemcpyHostToDevice));
  }

  ~TensorCuda() {
    if (m_data != nullptr) {
      CUDA_CHECK(cudaFree(m_data));
    }
  }

  TensorCuda(const TensorCuda &) = delete;

  TensorCuda &operator=(const TensorCuda &) = delete;

  TensorCuda(TensorCuda &&other) noexcept
      : m_data(std::exchange(other.m_data, nullptr)),
        m_rows(std::exchange(other.m_rows, 0)),
        m_cols(std::exchange(other.m_cols, 0)) {}

  TensorCuda &operator=(TensorCuda &&other) {
    if (this == &other) {
      return *this;
    }

    if (m_data != nullptr) {
      CUDA_CHECK(cudaFree(m_data));
    }

    m_data = std::exchange(other.m_data, nullptr);
    m_rows = std::exchange(other.m_rows, 0);
    m_cols = std::exchange(other.m_cols, 0);

    return *this;
  }

  [[nodiscard]]
  float *data() noexcept {
    return m_data;
  }

  [[nodiscard]]
  const float *data() const noexcept {
    return m_data;
  }

  [[nodiscard]]
  std::size_t rows() const noexcept {
    return m_rows;
  }

  [[nodiscard]]
  std::size_t cols() const noexcept {
    return m_cols;
  }

  [[nodiscard]]
  std::size_t size() const noexcept {
    return m_rows * m_cols;
  }

  [[nodiscard]]
  std::size_t sizeBytes() const noexcept {
    return size() * sizeof(float);
  }

private:
  float *m_data{nullptr};
  std::size_t m_rows{};
  std::size_t m_cols{};
};
