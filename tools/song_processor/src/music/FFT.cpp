#include "FFT.hpp"

#include <cmath>
#include <limits>
#include <new>
#include <numbers>
#include <stdexcept>

FFT::FFT(std::size_t size)
    : m_size(size),
      m_cfg(nullptr),
      m_windowed(size),
      m_bins(size / 2 + 1),
      m_hann(size)
{
    if (size < 2 ||
        size % 2 != 0 ||
        size > static_cast<std::size_t>(
            std::numeric_limits<int>::max())) {
        throw std::invalid_argument{
            "Invalid FFT size"
        };
    }

    m_cfg = kiss_fftr_alloc(
        static_cast<int>(size),
        0,
        nullptr,
        nullptr
    );

    if (m_cfg == nullptr) {
        throw std::bad_alloc{};
    }

    for (std::size_t i{}; i < size; ++i) {
        m_hann[i] = 0.5f * (1.0f - std::cos(
            (2.0f * std::numbers::pi_v<float> * i) /
            static_cast<float>(size - 1)
        ));
    }
}

FFT::~FFT()
{
    if (m_cfg != nullptr) {
        kiss_fftr_free(m_cfg);
    }
}

void FFT::transform(std::span<const float> samples, std::span<float> output)
{
    if (samples.size() != m_size) [[unlikely]] {
        throw std::invalid_argument{
            "Sample size does not match FFT size"
        };
    }

    for (std::size_t i{}; i < m_size; ++i) {
        m_windowed[i] = samples[i] * m_hann[i];
    }

    kiss_fftr(
        m_cfg,
        m_windowed.data(),
        m_bins.data()
    );

    for (std::size_t i{}; i < m_bins.size(); ++i) {
        output[i] =  std::hypot(m_bins[i].r, m_bins[i].i);
    }
}
