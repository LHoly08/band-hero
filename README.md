# band-hero

## License

Copyright (c) 2026 LHoly08. BandHero is licensed under the
GNU General Public License version 3 only (`GPL-3.0-only`). See [LICENSE](LICENSE)
for the full terms. Third-party code retains its own license notices.

## Game settings

Open Settings from the main menu. General and Gameplay changes save automatically;
sliders save when released. Instrument edits stay as drafts until you choose
**Save instrument**.

- **General:** drag master volume, choose the FPS limit and resolution, toggle
  fullscreen, and show an FPS counter throughout the game.
- **Custom Instruments:** create an instrument with a name and type, or select
  an existing instrument. Custom 1 edits section count and bits per section;
  Custom 2 edits effective bits for Easy and Hard, with Easy capped at Hard's
  count. Custom 3 edits the complete
  `PlayEasy`, `PlayHard`, and `Draw` Lua functions in separate tabs. The editor
  supports arrows, Home/End, mouse-wheel scrolling, and Ctrl+A/C/V. Invalid edits
  show an error and leave the last valid file intact. Save valid changes or choose
  **Revert edits** before leaving the section. Existing helper code is preserved.
  Names must be unique (capitalization and extra spaces are ignored). Use
  **Delete** to remove an instrument after confirmation. Existing duplicate
  names are flagged for renaming and excluded from player selection.
- **Gameplay:** select one of the six shared note colors and drag its RGB sliders.

Settings are stored in `settings.bin` and `startup.bin`, and custom instruments
in `Instruments/*.lua`, relative to the game's working directory. Created
instruments become available in player selection. Run the game from the
repository root to use the same files across launches.

Settings controls live under `include/ui/settings/` and `src/ui/settings/`;
persistence and instrument validation live under `include/config/` and
`src/config/`.

To build and run the isolated settings persistence tests:

```sh
cmake -S . -B build -DBANDHERO_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Python stem splitter

From `tools/song_processor/python/`, install into an activated Python virtual
environment:

```sh
python -m pip install .
python -m stem_splitter --name "Bored"
```

On Windows and Linux x86-64 with standard CPython 3.10–3.14, this installs
PyTorch 2.11.0 with CUDA 12.8 directly from the official PyTorch wheel host.
Linux requires glibc 2.28 or newer. Free-threaded Python builds are not supported
by these wheel references. Other platforms and Python versions use the default
PyTorch dependency, subject to upstream wheel availability.
The splitter automatically uses CUDA when available and otherwise uses the CPU.
CUDA execution requires a compatible NVIDIA GPU and driver; no separate CUDA
Toolkit installation is needed. The CUDA-enabled build is a larger download
even on machines that will run on CPU.

