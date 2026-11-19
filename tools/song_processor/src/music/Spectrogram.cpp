#include "Spectrogram.hpp"
#include "FFT.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <span>
#include <stdexcept>
#include <utility>
#include <kiss_fftr.h>

Tensor stft(const Audio& audio) {
    constexpr std::size_t FFTSize = STFTFFTSize;
    constexpr std::size_t HopSize = STFTHopSize;
    constexpr std::size_t Bins = FFTSize / 2 + 1;

    if (audio.samples.size() < FFTSize) [[unlikely]] {
        throw std::runtime_error("Samples size must be bigger than 2048!");
    }

    const std::size_t frames =
        1 + (audio.samples.size() - FFTSize) / HopSize;

    Tensor result{frames, Bins};

    FFT fft{FFTSize};

    for (std::size_t frame{}; frame < frames; ++frame) {
        const std::size_t offset = frame * HopSize;

        std::span<const float> input{
            audio.samples.data() + offset,
            FFTSize
        };

        std::span<float> magnitudes{
            result.data() + frame * Bins,
            Bins
        };
        fft.transform(input, magnitudes);
    }

    return result;
}
