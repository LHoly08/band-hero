#pragma once

#include "Tensor.hpp"

class Linear {
public:
    Linear(std::size_t inputSize, std::size_t outputSize);

    [[nodiscard]]
    Tensor forward(const Tensor &input);

    [[nodiscard]]
    Tensor backward(const Tensor &gradient, float learningRate);

private:
    void addBias(Tensor &output) const;

    Tensor m_weights;
    Tensor m_bias;

    Tensor m_input;
};