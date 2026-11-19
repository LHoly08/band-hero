#include "CrossEntropyCuda.cuh"

#include <cassert>

__global__ void crossEntropy(const float *probabilities,
                             const std::size_t *labels, float *loss,
                             const std::size_t rows, const std::size_t cols) {
  __shared__ float losses[256];

  const std::size_t index = threadIdx.x;
  const std::size_t row = index + blockIdx.x * blockDim.x;

  if (row >= rows) {
    losses[index] = 0.0f;
  } else {

    float p = probabilities[row * cols + labels[row]];

    if (p < 1e-7) {
      p = 1e-7;
    }

    losses[index] = -logf(p);
  }

  __syncthreads();

  std::size_t stride = 128;

  while (stride > 0) {

    if (index < stride) {
      losses[index] += losses[index + stride];
    }

    __syncthreads();

    stride /= 2;
  }

  if (index == 0) {
    loss[blockIdx.x] = losses[0];
  }
}

__global__ void reduce(const float *input, float *output,
                       const std::size_t count) {
  __shared__ float losses[256];

  const std::size_t index = threadIdx.x;

  const std::size_t row = index + blockIdx.x * blockDim.x;

  if (row >= count) {
    losses[index] = 0.0f;
  } else {
    losses[index] = input[row];
  }

  __syncthreads();

  std::size_t stride = 128;

  while (stride > 0) {

    if (index < stride) {
      losses[index] += losses[index + stride];
    }

    __syncthreads();

    stride /= 2;
  }

  if (index == 0) {
    output[blockIdx.x] = losses[0];
  }
}

float crossEntropyCuda(const TensorCuda &probabilities,
                       std::span<const std::size_t> labels) {

  assert(probabilities.rows() == labels.size());

  float *deviceLosses = nullptr;
  std::size_t *deviceLabels = nullptr;

  CUDA_CHECK(cudaMalloc(&deviceLabels, labels.size() * sizeof(std::size_t)));

  CUDA_CHECK(cudaMemcpy(deviceLabels, labels.data(),
                        labels.size() * sizeof(std::size_t),
                        cudaMemcpyHostToDevice));

  constexpr unsigned int threads = 256;
  const unsigned int blocks = (probabilities.rows() + threads - 1) / threads;
  unsigned int count = blocks;

  CUDA_CHECK(cudaMalloc(&deviceLosses, blocks * sizeof(float)));

  crossEntropy<<<blocks, threads>>>(probabilities.data(), deviceLabels,
                                    deviceLosses, probabilities.rows(),
                                    probabilities.cols());

  CUDA_CHECK(cudaGetLastError());

  float *deviceOutput = nullptr;

  while (count > 1) {

    const unsigned int grid = (count + threads - 1) / threads;

    CUDA_CHECK(cudaMalloc(&deviceOutput, grid * sizeof(float)));

    reduce<<<grid, threads>>>(deviceLosses, deviceOutput, count);

    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaFree(deviceLosses));
    deviceLosses = deviceOutput;
    deviceOutput = nullptr;

    count = grid;
  }

  CUDA_CHECK(cudaFree(deviceLabels));

  float loss{};

  CUDA_CHECK(
      cudaMemcpy(&loss, deviceLosses, sizeof(float), cudaMemcpyDeviceToHost));

  CUDA_CHECK(cudaFree(deviceLosses));

  return (loss / probabilities.rows());
}

__global__ void crossEntropyBack(const float *probabilities,
                                 const std::size_t *labels, float *gradient,
                                 const std::size_t rows,
                                 const std::size_t cols) {

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;
  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;

  if (row >= rows || col >= cols) {
    return;
  }
  const float batchSize = static_cast<float>(rows);

  gradient[row * cols + col] =
      (probabilities[row * cols + col] - ((col == labels[row]) ? 1.0f : 0.0f)) /
      batchSize;
}

TensorCuda crossEntropyBackwardCuda(const TensorCuda &probabilities,
                                    std::span<const std::size_t> labels) {

  assert(labels.size() == probabilities.rows());

  TensorCuda gradient{probabilities.rows(), probabilities.cols()};

  std::size_t *deviceLabels = nullptr;

  CUDA_CHECK(cudaMalloc(&deviceLabels, labels.size() * sizeof(std::size_t)));

  CUDA_CHECK(cudaMemcpy(deviceLabels, labels.data(),
                        labels.size() * sizeof(std::size_t),
                        cudaMemcpyHostToDevice));

  dim3 threads{16, 16};
  const unsigned int Nrows = (gradient.rows() + 15) / 16;
  const unsigned int Ncols = (gradient.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  crossEntropyBack<<<grid, threads>>>(probabilities.data(), deviceLabels,
                                      gradient.data(), gradient.rows(),
                                      gradient.cols());

  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaFree(deviceLabels));

  return gradient;
}
