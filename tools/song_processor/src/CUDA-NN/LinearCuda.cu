#include "LinearCuda.cuh"
#include "MatmulCuda.cuh"
#include "TransposeCuda.cuh"

#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>

Tensor makeWeights(std::size_t inputSize, std::size_t outputSize) {
  Tensor weights{inputSize, outputSize};

  std::mt19937 rng{std::random_device{}()};
  std::normal_distribution<float> dist{0.0f, 0.01f};

  for (std::size_t i = 0; i < inputSize; ++i) {
    for (std::size_t j = 0; j < outputSize; ++j) {
      weights(i, j) = dist(rng);
    }
  }

  return weights;
}

LinearCuda::LinearCuda(std::size_t inputSize, std::size_t outputSize)
    : m_weights(makeWeights(inputSize, outputSize)), m_bias(1, outputSize) {
  CUDA_CHECK(cudaMemset(m_bias.data(), 0, m_bias.sizeBytes()));
}

__global__ void addBias(float *output, const float *bias,
                        const std::size_t rows, const std::size_t cols) {

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;
  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;

  if (row >= rows || col >= cols) {
    return;
  }

  output[row * cols + col] += bias[col];
}

TensorCuda LinearCuda::forward(const TensorCuda &input) {

  m_input = &input;

  TensorCuda output = matmulCuda(input, m_weights);

  dim3 threads{16, 16};
  const unsigned int Nrows = (output.rows() + 15) / 16;
  const unsigned int Ncols = (output.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  addBias<<<grid, threads>>>(output.data(), m_bias.data(), output.rows(),
                             output.cols());

  CUDA_CHECK(cudaGetLastError());

  return output;
}

__global__ void biasGrad(const float *gradient, float *biasGradient,
                         const std::size_t cols, const std::size_t rows) {

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;

  if (col >= cols) {
    return;
  }

  for (std::size_t row{}; row < rows; ++row) {
    biasGradient[col] += gradient[row * cols + col];
  }
}

__global__ void learnWeights(const float *weightGradient, float *weights,
                             const float learningRate, const std::size_t rows,
                             const std::size_t cols) {

  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;
  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;

  if (row >= rows || col >= cols) {
    return;
  }

  weights[row * cols + col] -= learningRate * weightGradient[row * cols + col];
}

__global__ void learnBias(const float *biasGradient, float *bias,
                          const float learningRate, const std::size_t cols) {

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;

  if (col >= cols) {
    return;
  }

  bias[col] -= learningRate * biasGradient[col];
}

TensorCuda LinearCuda::backward(const TensorCuda &gradient,
                                float learningRate) {

  assert(m_input != nullptr);

  TensorCuda inputT = transposeCuda(*m_input);
  TensorCuda weightsT = transposeCuda(m_weights);

  TensorCuda weightGradient = matmulCuda(inputT, gradient);

  TensorCuda inputGradient = matmulCuda(gradient, weightsT);

  TensorCuda biasGradient{1, gradient.cols()};

  CUDA_CHECK(cudaMemset(biasGradient.data(), 0, biasGradient.sizeBytes()));

  {
    constexpr unsigned int threads = 256;
    const unsigned int blocks = (gradient.cols() + threads - 1) / threads;

    biasGrad<<<blocks, threads>>>(gradient.data(), biasGradient.data(),
                                  gradient.cols(), gradient.rows());

    CUDA_CHECK(cudaGetLastError());
  }
  {
    dim3 threads{16, 16};
    const unsigned int Nrows = (m_weights.rows() + 15) / 16;
    const unsigned int Ncols = (m_weights.cols() + 15) / 16;

    dim3 grid{Ncols, Nrows};
    learnWeights<<<grid, threads>>>(weightGradient.data(), m_weights.data(),
                                    learningRate, m_weights.rows(),
                                    m_weights.cols());

    CUDA_CHECK(cudaGetLastError());
  }
  {
    constexpr unsigned int threads = 256;
    const unsigned int blocks = (m_bias.cols() + threads - 1) / threads;

    learnBias<<<blocks, threads>>>(biasGradient.data(), m_bias.data(),
                                   learningRate, m_bias.cols());

    CUDA_CHECK(cudaGetLastError());
  }

  return inputGradient;
}

LinearCuda::LinearCuda(const Tensor &weights, const Tensor &bias)
    : m_weights(weights), m_bias(bias) {
  if (bias.rows() != 1 || bias.cols() != weights.cols()) {
    throw std::runtime_error{"Invalid LinearCuda dimensions"};
  }
}

void LinearCuda::save(std::ostream &out) const {
  Tensor weights{m_weights};
  Tensor bias{m_bias};

  weights.save(out);
  bias.save(out);
}

LinearCuda LinearCuda::load(std::istream &in) {
  Tensor weights = Tensor::load(in);

  Tensor bias = Tensor::load(in);

  return LinearCuda{weights, bias};
}