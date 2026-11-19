#include <exception>
#include <stdexcept>
#define DR_WAV_IMPLEMENTATION

#include "Audio.hpp"
#include "dr_wav.h"

Audio loadWav(const std::filesystem::path &path) {

    Audio audio{};

    drwav wav{};

    if (!drwav_init_file(&wav, path.string().c_str(), nullptr)) [[unlikely]] {
        throw std::runtime_error("Could not open WAV file: " + path.string());
    }

    std::size_t samples = wav.channels * wav.totalPCMFrameCount;

    std::vector<float> pcm(samples, 0.0f);

    if (wav.totalPCMFrameCount != drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pcm.data())) [[unlikely]] {
        drwav_uninit(&wav);
        throw std::runtime_error("Total PCM Frame Count is not the same as the read count!");
    }

    audio.sampleRate = wav.sampleRate;
    audio.samples.reserve(wav.totalPCMFrameCount);
        
    for (std::size_t i{}; i < samples; i += wav.channels) {
        float sum{};
        
        for (std::size_t j{}; j < wav.channels; ++j) {
            sum += pcm[i + j];
        }

        audio.samples.emplace_back(sum / static_cast<float>(wav.channels));
    }

    drwav_uninit(&wav);

    return audio;
}
