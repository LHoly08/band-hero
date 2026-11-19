#include "Tensor.hpp"
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>

void Tensor::save(std::ostream& out) const
{
    const auto rows =
        static_cast<std::uint64_t>(m_rows);

    const auto cols =
        static_cast<std::uint64_t>(m_cols);

    out.write(
        reinterpret_cast<const char*>(&rows),
        sizeof(rows)
    );

    out.write(
        reinterpret_cast<const char*>(&cols),
        sizeof(cols)
    );

    out.write(
        reinterpret_cast<const char*>(m_data.data()),
        static_cast<std::streamsize>(sizeBytes())
    );

    if (!out) {
        throw std::runtime_error{
            "Failed to save Tensor"
        };
    }
}

Tensor Tensor::load(std::istream& in)
{
    std::uint64_t rows{};
    std::uint64_t cols{};

    in.read(
        reinterpret_cast<char*>(&rows),
        sizeof(rows)
    );

    in.read(
        reinterpret_cast<char*>(&cols),
        sizeof(cols)
    );

    if (!in) {
        throw std::runtime_error{
            "Failed to read Tensor dimensions"
        };
    }

    Tensor tensor{
        static_cast<std::size_t>(rows),
        static_cast<std::size_t>(cols)
    };

    in.read(
        reinterpret_cast<char*>(tensor.data()),
        static_cast<std::streamsize>(
            tensor.sizeBytes()
        )
    );

    if (!in) {
        throw std::runtime_error{
            "Failed to read Tensor data"
        };
    }

    return tensor;
}
