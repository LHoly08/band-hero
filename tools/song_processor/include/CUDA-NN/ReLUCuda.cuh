#pragma once

#include "TensorCuda.cuh"

class ReLUCuda {
public:
  TensorCuda forward(const TensorCuda &input);
  TensorCuda backward(const TensorCuda &gradient) const;

private:
  const TensorCuda *m_input{nullptr};
};
