#include "SoftmaxCuda.cuh"

__global__ void softmax(const float *input, float *output,
                        const std ::size_t rows, const std::size_t cols) {
  const std::size_t row = blockIdx.x * blockDim.x + threadIdx.x;

  if (row >= rows) {
    return;
  }

  float max = input[row * cols];

  for (std::size_t col{1}; col < cols; ++col) {
    max = (max < input[row * cols + col]) ? input[row * cols + col] : max;
  }
  float sum{};

  for (std::size_t col{}; col < cols; ++col) {
    output[row * cols + col] = expf(input[row * cols + col] - max);
    sum += output[row * cols + col];
  }

  for (std::size_t col{}; col < cols; ++col) {
    output[row * cols + col] /= sum;
  }
}

TensorCuda softmaxCuda(const TensorCuda &input) {

  TensorCuda output{input.rows(), input.cols()};

  constexpr unsigned int threads = 256;
  const unsigned int blocks = (input.rows() + threads - 1) / threads;

  softmax<<<blocks, threads>>>(input.data(), output.data(), input.rows(),
                               input.cols());

  CUDA_CHECK(cudaGetLastError());

  return output;
}