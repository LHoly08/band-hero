#pragma once

#include <cstddef>
#include <vector>
#include <ostream>
#include <istream>

class TensorCuda;

class Tensor {
public:
    Tensor(std::size_t rows, std::size_t cols)
        : m_data(rows * cols),
          m_rows(rows),
          m_cols(cols)
    {}

    Tensor(const TensorCuda& tensor);

    Tensor(const Tensor&) = default;
    Tensor& operator=(const Tensor&) = default;

    Tensor(Tensor&&) noexcept = default;
    Tensor& operator=(Tensor&&) noexcept = default;

    ~Tensor() = default;

    float& operator()(std::size_t row, std::size_t col) noexcept
    {
        return m_data[row * m_cols + col];
    }

    const float& operator()(
        std::size_t row,
        std::size_t col) const noexcept
    {
        return m_data[row * m_cols + col];
    }

    void save(std::ostream& out) const;

    [[nodiscard]]
    static Tensor load(std::istream& in);

    [[nodiscard]]
    float* data() noexcept
    {
        return m_data.data();
    }

    [[nodiscard]]
    const float* data() const noexcept
    {
        return m_data.data();
    }

    [[nodiscard]]
    std::size_t rows() const noexcept
    {
        return m_rows;
    }

    [[nodiscard]]
    std::size_t cols() const noexcept
    {
        return m_cols;
    }

    [[nodiscard]]
    std::size_t size() const noexcept
    {
        return m_rows * m_cols;
    }

    [[nodiscard]]
    std::size_t sizeBytes() const noexcept
    {
        return size() * sizeof(float);
    }

private:
    std::vector<float> m_data;
    std::size_t m_rows;
    std::size_t m_cols;
};