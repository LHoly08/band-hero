#include "Tensor.hpp"
#include "TensorCuda.cuh"
#include "CudaCheck.cuh"

Tensor::Tensor(const TensorCuda& tensor)
    : Tensor(tensor.rows(), tensor.cols())
{
    CUDA_CHECK(cudaMemcpy(
        m_data.data(),
        tensor.data(),
        sizeBytes(),
        cudaMemcpyDeviceToHost
    ));
}
