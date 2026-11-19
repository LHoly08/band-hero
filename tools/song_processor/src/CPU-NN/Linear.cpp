#include "Linear.hpp"
#include "Matmul.hpp"
#include "Transpose.hpp"

#include <cstddef>
#include <random>

Linear::Linear(std::size_t inputSize, std::size_t outputSize)
    : m_weights(inputSize, outputSize),
          m_bias(1, outputSize), m_input(0, 0)
    {
        std::mt19937 rng{std::random_device{}()};
        std::normal_distribution<float> dist{
            0.0f,
            0.01f
        };

        for (std::size_t i = 0; i < inputSize; ++i) {
            for (std::size_t j = 0; j < outputSize; ++j) {
                m_weights(i, j) = dist(rng);
            }
        }
    }

Tensor Linear::forward(const Tensor &input) {

    m_input = input;

    Tensor output = matmul(input, m_weights);
    addBias(output);

    return output;
}

Tensor Linear::backward(const Tensor &gradient, float learningRate) {

    Tensor inputT = transpose(m_input);
    Tensor weightsT = transpose(m_weights);

    Tensor weightGradient =
        matmul(inputT, gradient);

    Tensor inputGradient =
        matmul(gradient, weightsT);

    Tensor biasGradient{1, gradient.cols()};

    for (std::size_t row{}; row < gradient.rows(); ++row) {
        for (std::size_t col{}; col < gradient.cols(); ++col) {
            biasGradient(0, col) += gradient(row, col);
        }
    }

    for (std::size_t row{}; row < m_weights.rows(); ++row) {
        for (std::size_t col{}; col < m_weights.cols(); ++col) {
            m_weights(row, col) -=
                learningRate * weightGradient(row, col);
        }
    }

    for (std::size_t col{}; col < m_bias.cols(); ++col) {
        m_bias(0, col) -=
            learningRate * biasGradient(0, col);
    }

    return inputGradient;
}

void Linear::addBias(Tensor &output) const {
    for (size_t i{}; i < output.rows(); ++i) {
        for (size_t j{}; j < m_bias.cols(); ++j) {
            output(i, j) += m_bias(0, j);
        }
    }
}