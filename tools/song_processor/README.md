# Band Hero song processor

Use the Python stem splitter to produce `drums.wav`, then run the C++ song
processor to export candidate drum-onset times. The C++ tool builds
independently of the game.

## Build the song processor

Start in `tools/song_processor`. Use CMake 3.31 or newer, Ninja, and a C++23
compiler with `std::print`. WAV analysis runs on the CPU.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSONG_PROCESSOR_ENABLE_CUDA=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

## Split a song

Skip this step if you already have a drum WAV. From `tools/song_processor`,
install the splitter in a virtual environment and pass the song name without
its extension. The default input extension is `mp3`.

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install ./python
.\.venv\Scripts\python.exe -m stem_splitter --path "D:\audio" --name "Bored"
```

This reads `D:\audio\Bored.mp3` and writes the stems under
`Songs/Bored/Audio/` in the working directory. For a different input
extension, pass `--ftype wav` or `--ftype flac`. On Linux, use
`.venv/bin/python` for the virtual environment commands.

On x86-64 Windows and Linux, standard CPython 3.10–3.14 installs the pinned
PyTorch CUDA wheel. Linux requires glibc 2.28 or newer. Other platforms and
Python versions use the default PyTorch dependency where available. Demucs
uses a compatible NVIDIA GPU when PyTorch detects one and otherwise runs on
the CPU. The pinned wheel includes its CUDA runtime.

## Analyze the drum stem

```powershell
.\build\song_processor.exe flux "Songs/Bored/Audio/drums.wav"
.\build\song_processor.exe flux "Songs/Bored/Audio/drums.wav" --raw --margin 5
```

On Linux, run `./build/song_processor`. The processor averages the WAV channels
to mono and analyzes complete 2048-sample Hann windows with a 512-sample hop.
The default flux uses log-compressed magnitudes. `--raw` uses linear magnitudes
and `--margin` raises the onset threshold.

Each run replaces two files in the working directory.

| File | Contents |
| --- | --- |
| `flux.csv` | Frame index, window-start time in seconds, and spectral flux |
| `onsets.csv` | Candidate drum-onset window-start times in seconds |

## Run the CUDA MNIST experiment

The CUDA neural network classifies handwritten digits. The drum-onset
command uses spectral flux.

On Windows, open an x64 Visual Studio Developer PowerShell with Ninja and
the CUDA Toolkit available. From `tools/song_processor`, build the CUDA
version in a separate directory.

```powershell
cmake -S . -B build-cuda -G Ninja -DCMAKE_BUILD_TYPE=Release -DSONG_PROCESSOR_ENABLE_CUDA=ON
cmake --build build-cuda
```

Run the executable from a working directory with `mnist.lhnn` and a
`data/` folder containing the uncompressed MNIST IDX files. The `test`
command requires `t10k-images-idx3-ubyte` and `t10k-labels-idx1-ubyte`.
The `train` command also requires `train-images-idx3-ubyte` and
`train-labels-idx1-ubyte`. It resumes from the checkpoint when present
and saves back to `mnist.lhnn`.

```powershell
.\build-cuda\song_processor.exe test
.\build-cuda\song_processor.exe train
```

Keep local songs, generated stems and CSVs, build directories, Python
environments, the MNIST dataset, and model checkpoints out of Git.

## Source layout

| Directory | Contents |
| --- | --- |
| `src/CPU-NN/` and `include/CPU-NN/` | Host tensors, math, and CPU neural network layers |
| `src/CUDA-NN/` and `include/CUDA-NN/` | CUDA kernels, model, and MNIST commands |
| `src/music/` and `include/music/` | WAV decoding, FFT, spectral flux, and onset detection |
| `src/main.cpp` | Command-line entry point |
| `python/` | Demucs stem splitter |
| `tests/` | Spectral flux tests |
| `third_party/` | FFT and WAV decoding dependencies |

## Third-party code

The tool builds the real FFT from KissFFT 131.2.0, upstream commit
`7bce4153c6bc8aba2db0e889e576f9d00505cbe1`. Its license is in
[`third_party/kissfft/COPYING`](third_party/kissfft/COPYING).
The WAV decoder uses [`dr_wav.h`](third_party/dr_wav/dr_wav.h), which
includes its license.
