#include "ReLU.hpp"
#include "Tensor.hpp"

Tensor ReLU::forward(const Tensor &input) {

    m_input = input;
    Tensor output = input;

    for (std::size_t row{}; row < output.rows(); ++row) {
        for (std::size_t col{}; col < output.cols(); ++col) {

            output(row, col) = std::max(0.0f, output(row, col));
        }
    }
    return output;
}

Tensor ReLU::backward(const Tensor &gradient) const {

    Tensor output = gradient;

    for (std::size_t row{}; row < output.rows(); ++row) {
        for (std::size_t col{}; col < output.cols(); ++col) {
            
            const bool condition = m_input(row, col) <= 0.0f;
            output(row, col) =  output(row, col) * !condition;
        }
    }
    return output;
}