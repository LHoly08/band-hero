#include "Transpose.hpp"

Tensor transpose(const Tensor &input) {

    Tensor output{input.cols(), input.rows()};

    for (std::size_t row{}; row < input.rows(); ++row) {
        for (std::size_t col{}; col < input.cols(); ++col) {
            output(col, row) = input(row, col);
        }
    }

    return output;
}