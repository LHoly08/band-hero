#include "CrossEntropy.hpp"

#include <cassert>
#include <cmath>

float crossEntropy(
    const Tensor &probabilities,
    std::span<const std::size_t> labels) {

    assert(probabilities.rows() == labels.size());

    float loss{};

    for (std::size_t row{}; row < probabilities.rows(); ++row) {
        const float p = std::max(
            probabilities(row, labels[row]),
            1e-7f
        );

        loss -= std::log(p);
    }

    return loss / static_cast<float>(labels.size());
}

Tensor crossEntropyBackward(const Tensor &probabilities, std::span<const std::size_t> labels) {

    Tensor gradient = probabilities;

    for (std::size_t row{}; row < gradient.rows(); ++row) {
        gradient(row, labels[row]) -= 1.0f;
    }

    const float batchSize =
        static_cast<float>(gradient.rows());

    for (std::size_t row{}; row < gradient.rows(); ++row) {
        for (std::size_t col{}; col < gradient.cols(); ++col){
            gradient(row, col) /= batchSize;
        }
    }

    return gradient;

}