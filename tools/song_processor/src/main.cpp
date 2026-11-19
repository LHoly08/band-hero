#include "Audio.hpp"
#include "DetectOnsets.hpp"
#include "Spectrogram.hpp"
#include "SpectralFlux.hpp"
#include "Tensor.hpp"

#ifdef SONG_PROCESSOR_ENABLE_CUDA
#include "MnistCommands.hpp"
#endif

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <print>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

void fluxTest(const std::filesystem::path& wavPath, bool useLogCompression, float margin)
{
    const Audio audio = loadWav(wavPath);
    if (audio.sampleRate == 0) {
        throw std::runtime_error{"WAV sample rate must be positive"};
    }

    const Tensor spectrogram = stft(audio);
    const auto flux = spectralFlux(spectrogram, useLogCompression);
    const auto onsets = detectOnsets(flux, audio.sampleRate, margin);
    const auto frameTime = [&audio](std::size_t frame) {
        // Window-start time, not an exact drum-hit time.
        return static_cast<double>(frame) * STFTHopSize / audio.sampleRate;
    };

    std::ofstream csv{"flux.csv"};
    if (!csv) {
        throw std::runtime_error{"Could not open flux.csv for writing"};
    }
    csv.imbue(std::locale::classic());
    csv << std::setprecision(std::numeric_limits<double>::max_digits10);
    csv << "frame,time_seconds,flux\n";
    for (std::size_t frame{}; frame < flux.size(); ++frame) {
        csv << frame << ',' << frameTime(frame) << ',' << flux[frame] << '\n';
    }
    csv.close();
    if (!csv) {
        throw std::runtime_error{"Could not finish writing flux.csv"};
    }

    std::ofstream onsetCsv{"onsets.csv"};
    if (!onsetCsv) {
        throw std::runtime_error{"Could not open onsets.csv for writing"};
    }
    onsetCsv.imbue(std::locale::classic());
    onsetCsv << std::setprecision(std::numeric_limits<double>::max_digits10);
    onsetCsv << "time_seconds\n";
    for (const double time : onsets) {
        onsetCsv << time << '\n';
    }
    onsetCsv.close();
    if (!onsetCsv) {
        throw std::runtime_error{"Could not finish writing onsets.csv"};
    }

    const auto maximum = std::max_element(flux.begin(), flux.end());
    const auto maxFrame = static_cast<std::size_t>(maximum - flux.begin());
    std::println("Frames: {}", spectrogram.rows());
    std::println("Frequency bins: {}", spectrogram.cols());
    std::println("Sample rate: {} Hz", audio.sampleRate);
    std::println("Audio duration: {:.2f}s",
        static_cast<double>(audio.samples.size()) / audio.sampleRate);
    std::println("Flux magnitudes: {}", useLogCompression ? "log1p" : "raw");
    std::println("Maximum flux: {:.6f}, frame: {}, window-start time: {:.6f}s",
        *maximum, maxFrame, frameTime(maxFrame));
    std::println("Saved flux.csv (maximum flux is a diagnostic, not a detected hit).");
    std::println("Candidate onsets: {} (margin: {})", onsets.size(), margin);
    std::println("First {} candidate window-start times:", std::min(onsets.size(), std::size_t{20}));
    for (std::size_t i{}; i < std::min(onsets.size(), std::size_t{20}); ++i) {
        std::println("  {:.6f}s", onsets[i]);
    }
    std::println("Saved all candidates to onsets.csv.");
}

void printUsage()
{
    std::println("Usage:\n"
#ifdef SONG_PROCESSOR_ENABLE_CUDA
        "  song_processor train\n"
        "  song_processor test\n"
#endif
        "  song_processor flux <wav-path> [--raw] [--margin <nonnegative-number>]");
}

} // namespace

int main(int argc, char* argv[])
{
    try {
        if (argc >= 2) {
            const std::string_view mode{argv[1]};
#ifdef SONG_PROCESSOR_ENABLE_CUDA
            if (mode == "train" && argc == 2) {
                trainMnist();
                return 0;
            }
            if (mode == "test" && argc == 2) {
                testMnist();
                return 0;
            }
#endif
            if (mode == "flux" && argc >= 3) {
                bool useLogCompression = true;
                float margin = 0.0f;
                for (int i = 3; i < argc; ++i) {
                    const std::string_view option{argv[i]};
                    if (option == "--raw") {
                        useLogCompression = false;
                    } else if (option == "--margin" && i + 1 < argc) {
                        const std::string_view value{argv[++i]};
                        const auto parsed = std::from_chars(
                            value.data(), value.data() + value.size(), margin);
                        if (parsed.ec != std::errc{} ||
                            parsed.ptr != value.data() + value.size() ||
                            !std::isfinite(margin) || margin < 0.0f) {
                            throw std::invalid_argument{"Margin must be a finite nonnegative number"};
                        }
                    } else {
                        printUsage();
                        return 1;
                    }
                }
                fluxTest(argv[2], useLogCompression, margin);
                return 0;
            }
        }
        printUsage();
        return 1;
    } catch (const std::exception& error) {
        std::println(stderr, "Error: {}", error.what());
        return 1;
    }
}
