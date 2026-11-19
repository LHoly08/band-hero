#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <kiss_fftr.h>

class FFT {
public:
    explicit FFT(std::size_t size);

    ~FFT();

    FFT(const FFT&) = delete;
    FFT& operator=(const FFT&) = delete;

    FFT(FFT&&) = delete;
    FFT& operator=(FFT&&) = delete;

    void transform(std::span<const float> samples, std::span<float> output);

private:
    std::size_t m_size;

    kiss_fftr_cfg m_cfg{nullptr};

    std::vector<float> m_windowed;
    std::vector<kiss_fft_cpx> m_bins;
    std::vector<float> m_hann;
};
