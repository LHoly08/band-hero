#pragma once

#include "Tensor.hpp"

class ReLU {
public: 
    Tensor forward(const Tensor &input);

    Tensor backward(const Tensor &gradient) const;

private:
    Tensor m_input{0, 0};
};