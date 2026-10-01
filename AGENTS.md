# BandHero agent notes

## Project layout

- `src/` and `include/` contain the C++ game code.
- `assets/` contains game assets, which CMake copies beside the built executable.
- `tools/song_processor/python/` contains the installable Python stem splitter.

## Build and run

- Use CMake 3.31 or newer and a C++26 compiler that supports `-freflection`.
- Build with `cmake --build build --parallel` after configuring the build directory.
- Run `./build/BandHero` from the repository root in a graphical session.
- Do not edit generated files in `build/`.

## Validation

- Build `BandHero` after changing C++ code or CMake files.
- For Python changes, work from `tools/song_processor/python/` and run the splitter
  as a package (`python -m stem_splitter`) or through its installed
  `stem-splitter` command.

## Editing

- Check `git status` before editing and preserve existing work in progress.
- Keep asset paths in code consistent with the files under `assets/`.
