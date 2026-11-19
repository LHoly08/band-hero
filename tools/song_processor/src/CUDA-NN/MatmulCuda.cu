#include "CudaCheck.cuh"
#include "MatmulCuda.cuh"

#include <cassert>

__global__ void mulMatTiled(const float *a, const float *b, float *c,
                            const std::size_t colsA, const std::size_t colsB,
                            const std::size_t rowsC,
                            const std::size_t tileCount) {

  __shared__ float tileA[16][16];
  __shared__ float tileB[16][16];

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;
  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;

  float val{};

  for (std::size_t tile{}; tile < tileCount; ++tile) {

    if ((tile * 16) + threadIdx.x < colsA && row < rowsC) {
      tileA[threadIdx.y][threadIdx.x] =
          a[row * colsA + (tile * 16) + threadIdx.x];
    } else {
      tileA[threadIdx.y][threadIdx.x] = 0.0f;
    }
    if ((tile * 16) + threadIdx.y < colsA && col < colsB) {
      tileB[threadIdx.y][threadIdx.x] =
          b[((tile * 16) + threadIdx.y) * colsB + col];
    } else {
      tileB[threadIdx.y][threadIdx.x] = 0.0f;
    }

    __syncthreads();

    for (std::size_t k{}; k < 16; ++k) {
      val += tileA[threadIdx.y][k] * tileB[k][threadIdx.x];
    }

    __syncthreads();
  }

  if (col < colsB && row < rowsC) {
    c[row * colsB + col] = val;
  }
}

__global__ void mulMat(const float *a, const float *b, float *c,
                       const std::size_t colsA, const std::size_t colsB,
                       const std::size_t rowsC) {

  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;
  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;

  if (row >= rowsC || col >= colsB) {
    return;
  }

  float val{};
  for (std::size_t i{}; i < colsA; ++i) {
    val += a[row * colsA + i] * b[i * colsB + col];
  }
  c[row * colsB + col] = val;
}

Tensor matmulCuda(const Tensor &a, const Tensor &b) {

  assert(a.cols() == b.rows());

  Tensor output{a.rows(), b.cols()};

  dim3 threads{16, 16};
  const unsigned int Nrows = (output.rows() + 15) / 16;
  const unsigned int Ncols = (output.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  float *deviceA = nullptr;
  float *deviceB = nullptr;
  float *deviceC = nullptr;

  const std::size_t sizeA = (a.rows() * a.cols()) * sizeof(float);
  const std::size_t sizeB = (b.rows() * b.cols()) * sizeof(float);
  const std::size_t sizeC = (output.rows() * output.cols()) * sizeof(float);

  CUDA_CHECK(cudaMalloc(&deviceA, sizeA));
  CUDA_CHECK(cudaMalloc(&deviceB, sizeB));
  CUDA_CHECK(cudaMalloc(&deviceC, sizeC));

  CUDA_CHECK(cudaMemcpy(deviceA, a.data(), sizeA, cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(deviceB, b.data(), sizeB, cudaMemcpyHostToDevice));

  mulMat<<<grid, threads>>>(deviceA, deviceB, deviceC, a.cols(), b.cols(),
                            output.rows());

  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaDeviceSynchronize());

  CUDA_CHECK(cudaMemcpy(output.data(), deviceC, sizeC, cudaMemcpyDeviceToHost));

  CUDA_CHECK(cudaFree(deviceA));
  CUDA_CHECK(cudaFree(deviceB));
  CUDA_CHECK(cudaFree(deviceC));

  return output;
}

TensorCuda matmulCuda(const TensorCuda &a, const TensorCuda &b) {

  assert(a.cols() == b.rows());

  TensorCuda output{a.rows(), b.cols()};

  dim3 threads{16, 16};
  const unsigned int Nrows = (output.rows() + 15) / 16;
  const unsigned int Ncols = (output.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  mulMat<<<grid, threads>>>(a.data(), b.data(), output.data(), a.cols(),
                            b.cols(), output.rows());

  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaDeviceSynchronize());

  return output;
}

TensorCuda matmulTiled(const TensorCuda &a, const TensorCuda &b) {

  assert(a.cols() == b.rows());

  TensorCuda output{a.rows(), b.cols()};

  dim3 threads{16, 16};
  const unsigned int Nrows = (output.rows() + 15) / 16;
  const unsigned int Ncols = (output.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  const std::size_t tileCount = std::ceil(a.cols() / 16.f);

  mulMatTiled<<<grid, threads>>>(a.data(), b.data(), output.data(), a.cols(),
                                 b.cols(), output.rows(), tileCount);

  CUDA_CHECK(cudaGetLastError());

  CUDA_CHECK(cudaDeviceSynchronize());

  return output;
}
