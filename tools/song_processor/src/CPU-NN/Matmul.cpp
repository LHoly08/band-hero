#include "Matmul.hpp"
#include <cassert>


Tensor matmul(const Tensor &a, const Tensor &b) {

    assert(a.cols() == b.rows());

    Tensor output{a.rows(), b.cols()};

    for (std::size_t row{}; row < a.rows(); ++row) {
        for (std::size_t col{}; col < b.cols(); ++col) {

            for (std::size_t i = 0; i < a.cols(); ++i) {
                output(row, col) += a(row, i) * b(i, col);
            }
        }
    }

    return output;
}