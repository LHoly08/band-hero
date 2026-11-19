#include "DetectOnsets.hpp"
#include <ranges>
#include <tuple>
#include "Spectrogram.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

std::vector<double> detectOnsets(std::span<const float> flux, std::uint32_t sampleRate, float margin) {

    if (sampleRate == 0) {
        throw std::invalid_argument{"Sample rate must be positive"};
    }

    if (!std::isfinite(margin) || margin < 0.0f) {
        throw std::invalid_argument{"Margin must be finite and nonnegative"};
    }

    if (flux.size() < 3) {
        return {};
    }

    const double secondsPerFrame =
        static_cast<double>(STFTHopSize) / sampleRate;

    const auto radius = static_cast<std::size_t>(
        std::ceil(0.100 / secondsPerFrame));

    const auto minSeparation = static_cast<std::size_t>(
        std::ceil(0.030 / secondsPerFrame));

    std::vector<std::size_t> selectedFrames;

    for (std::size_t frame{1}; frame + 1 < flux.size(); ++frame) {
        if (flux[frame] <= flux[frame - 1] || flux[frame] < flux[frame + 1]) {
            continue;
        }

        const std::size_t begin =
            frame > radius ? frame - radius : 0;

        const std::size_t end =
            frame + std::min(radius, flux.size() - 1 - frame) + 1;

        double sum = 0.0;

        for (std::size_t i = begin; i < end; ++i) {
            sum += flux[i];
        }

        const double average =
            sum / static_cast<double>(end - begin);

        if (flux[frame] <= average + margin) {
            continue;
        }

        if (!selectedFrames.empty()) {
            const std::size_t previous = selectedFrames.back();

            if (frame - previous < minSeparation) {
                // Replace the previous peak only if this one is stronger.
                if (flux[frame] > flux[previous]) {
                    selectedFrames.back() = frame;
                }

                continue;
            }
        }

        selectedFrames.push_back(frame);
    }

    std::vector<double> onsets(selectedFrames.size());

    for (auto &&[frame, time] : std::ranges::views::zip(selectedFrames, onsets)) {
        time = static_cast<double>(frame) * STFTHopSize / sampleRate;
    }

    return onsets;
}