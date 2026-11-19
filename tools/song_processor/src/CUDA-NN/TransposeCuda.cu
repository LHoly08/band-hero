#include "TransposeCuda.cuh"

__global__ void transpose(const float *input, float *output,
                          const std::size_t inputCols,
                          const std::size_t outputCols) {

  const std::size_t row = threadIdx.y + blockIdx.y * blockDim.y;
  const std::size_t col = threadIdx.x + blockIdx.x * blockDim.x;

  if (row >= outputCols || col >= inputCols) {
    return;
  }

  output[col * outputCols + row] = input[row * inputCols + col];
}

TensorCuda transposeCuda(const TensorCuda &input) {

  TensorCuda output{input.cols(), input.rows()};

  dim3 threads{16, 16};
  const unsigned int Nrows = (input.rows() + 15) / 16;
  const unsigned int Ncols = (input.cols() + 15) / 16;

  dim3 grid{Ncols, Nrows};

  transpose<<<grid, threads>>>(input.data(), output.data(), input.cols(),
                               output.cols());

  CUDA_CHECK(cudaGetLastError());

  return output;
}
