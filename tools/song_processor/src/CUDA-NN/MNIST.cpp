#include "MNIST.hpp"

#include <bit>
#include <cstdint>
#include <fstream>
#include <stdexcept>

std::uint32_t readU32(std::ifstream& file)
{
    std::uint32_t value{};

    file.read(
        reinterpret_cast<char*>(&value),
        sizeof(value)
    );

    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}

MNIST loadMNIST(
    const std::filesystem::path& imagesPath,
    const std::filesystem::path& labelsPath)
{
    std::ifstream imageFile{imagesPath, std::ios::binary};
    std::ifstream labelFile{labelsPath, std::ios::binary};

    if (!imageFile || !labelFile) {
        throw std::runtime_error{"Failed to open MNIST files"};
    }

    const auto imageMagic = readU32(imageFile);
    const auto imageCount = readU32(imageFile);
    const auto rows = readU32(imageFile);
    const auto cols = readU32(imageFile);

    const auto labelMagic = readU32(labelFile);
    const auto labelCount = readU32(labelFile);

    if (imageMagic != 2051) {
        throw std::runtime_error{"Invalid MNIST image file"};
    }

    if (labelMagic != 2049) {
        throw std::runtime_error{"Invalid MNIST label file"};
    }

    if (imageCount != labelCount) {
        throw std::runtime_error{"Image/label count mismatch"};
    }

    Tensor images{
        imageCount,
        rows * cols
    };

    std::vector<std::size_t> labels(imageCount);
    
    for (std::size_t image{}; image < imageCount; ++image) {
        for (std::size_t pixel{}; pixel < rows * cols; ++pixel) {

            std::uint8_t value;
            imageFile.read(
                reinterpret_cast<char*>(&value),
                sizeof(value)
            );

            images(image, pixel) =
                static_cast<float>(value) / 255.0f;
        }
    }

    for (std::size_t i{}; i < labelCount; ++i) {
        std::uint8_t label;

        labelFile.read(
            reinterpret_cast<char*>(&label),
            sizeof(label)
        );

        labels[i] = label;
    }

    return {
        std::move(images),
        std::move(labels)
    };
}

Tensor getBatch(
    const Tensor& dataset,
    std::size_t offset,
    std::size_t count)
{
    Tensor batch{count, dataset.cols()};

    for (std::size_t row{}; row < count; ++row) {
        for (std::size_t col{}; col < dataset.cols(); ++col) {
            batch(row, col) =
                dataset(offset + row, col);
        }
    }

    return batch;
}

std::vector<std::size_t> getLabels(
    const std::vector<std::size_t>& labels,
    std::size_t offset,
    std::size_t count)
{
    return {
        labels.begin() + offset,
        labels.begin() + offset + count
    };
}