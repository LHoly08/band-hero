#include "Softmax.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

Tensor softmax(const Tensor& input)
{
    Tensor output{input.rows(), input.cols()};

    for (std::size_t row{}; row < input.rows(); ++row) {

        float max = -std::numeric_limits<float>::infinity();

        for (std::size_t col{}; col < input.cols(); ++col)
            max = std::max(max, input(row, col));

        float sum{};

        for (std::size_t col{}; col < input.cols(); ++col) {
            output(row, col) = std::exp(input(row, col) - max);
            sum += output(row, col);
        }

        for (std::size_t col{}; col < input.cols(); ++col)
            output(row, col) /= sum;
    }

    return output;
}