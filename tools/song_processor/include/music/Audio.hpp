#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

struct Audio {
    std::vector<float> samples;
    std::uint32_t sampleRate;
};

Audio loadWav(const std::filesystem::path& path);