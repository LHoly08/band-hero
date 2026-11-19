#include "SpectralFlux.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

std::vector<float> spectralFlux(
    const Tensor& spectrogram,
    bool useLogCompression)
{
    std::vector<float> flux(spectrogram.rows(), 0.0f);

    for (std::size_t frame{1};
         frame < spectrogram.rows();
         ++frame)
    {
        float sum{};

        for (std::size_t bin{};
             bin < spectrogram.cols();
             ++bin)
        {
            float current = spectrogram(frame, bin);
            float previous = spectrogram(frame - 1, bin);

            if (useLogCompression) {
                current = std::log1p(current);
                previous = std::log1p(previous);
            }

            sum += std::max(0.0f, current - previous);
        }

        flux[frame] = sum;
    }

    return flux;
}