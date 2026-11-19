#pragma once

#include "Tensor.hpp"
#include <filesystem>

struct MNIST {
    Tensor images;
    std::vector<std::size_t> labels;
};

MNIST loadMNIST(
    const std::filesystem::path& imagesPath,
    const std::filesystem::path& labelsPath
);

Tensor getBatch(
    const Tensor& dataset,
    std::size_t offset,
    std::size_t count);

std::vector<std::size_t> getLabels(
    const std::vector<std::size_t>& labels,
    std::size_t offset,
    std::size_t count);