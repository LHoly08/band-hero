#pragma once

#include <vector>
#include <span>
#include <cstdint>

std::vector<double> detectOnsets(std::span<const float> flux, std::uint32_t sampleRate, float margin = 5.0f);