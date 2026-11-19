#include "ReLUCuda.cuh"
#include <cassert>

__global__ void relu(const float *input, float *output,
                     const std::size_t size) {

  const std::size_t index = threadIdx.x + blockIdx.x * blockDim.x;

  if (index >= size) {
    return;
  }

  output[index] = (input[index] > 0.0f) ? input[index] : 0.0f;
}

TensorCuda ReLUCuda::forward(const TensorCuda &input) {

  m_input = &input;

  TensorCuda output{input.rows(), input.cols()};

  constexpr unsigned int threads = 256;
  const unsigned int blocks = (input.size() + threads - 1) / threads;

  relu<<<blocks, threads>>>(input.data(), output.data(), input.size());

  CUDA_CHECK(cudaGetLastError());

  return output;
}

__global__ void reluBack(const float *gradient, const float *input,
                         float *output, const std::size_t rows,
                         const std::size_t cols) {

  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;
  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;

  if (row >= rows || col >= cols) {
    return;
  }

  const bool condition = (input[row * cols + col] <= 0.0f);
  output[row * cols + col] = gradient[row * cols + col] * !condition;
}

TensorCuda ReLUCuda::backward(const TensorCuda &gradient) const {
  assert(m_input != nullptr);

  TensorCuda output{gradient.rows(), gradient.cols()};

  dim3 threads{16, 16};
  const unsigned int Nrows = (output.rows() + 15) / 16;
  const unsigned int Ncols = (output.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  reluBack<<<grid, threads>>>(gradient.data(), m_input->data(), output.data(),
                              output.rows(), output.cols());

  CUDA_CHECK(cudaGetLastError());

  return output;
}
