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
  fullscreen, and show an FPS counter throughout the game. Choose the serial
  controller port and baud rate; use **Refresh** after connecting a device.
  Choose **None** to disable the serial controller. Serial changes save automatically
  and apply when entering player selection or gameplay.
- **Custom Instruments:** create an instrument with a name and type, or select
  an existing instrument. Custom 1 edits section count and bits per section;
  Custom 2 edits effective bits for Easy and Hard, with Easy capped at Hard's
  count. Custom 3 edits the complete
  `PlayEasy`, `PlayHard`, and `Draw` Lua functions in separate tabs. The editor
  supports arrows, Home/End, mouse-wheel scrolling, and Ctrl+A/C/V. Invalid edits
  show an error and leave the last valid file intact. Switching to General,
  Gameplay, or Main Menu discards unsaved instrument edits automatically.
  **Revert edits** also discards edits while staying in the editor. Existing helper code is preserved.
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

During gameplay, missing a chart note lowers that player's instrument stem to
25% volume. The next successfully completed note or chord restores full volume.
The stem keeps playing in time with the song, and the master volume setting
still applies.

## Controller firmware protocol

Send a complete input state whenever it changes, including releases. Use the
configured serial baud rate and write exactly four binary bytes per packet,
least significant byte first:

```cpp
uint32_t packet = (state << 2) | playerId; // playerId: 0-3, state: 30 bits
for (unsigned i = 0; i < 4; ++i)
    Serial.write(static_cast<uint8_t>(packet >> (8 * i)));
```

Communication is one-way: instruments send packets and the game only reads.
The lowest two packet bits are the player ID (`packet & 3`); the remaining
bits are the full input state (`packet >> 2`). No handshake is required and
the first received press is processed immediately. Do not send diagnostic
text on this serial connection.

On entering gameplay, including after pause, the game clears queued serial
data and resets remembered inputs to the released state.
Release controls before starting or resuming play.

Poll and debounce inputs in the firmware loop. Send
only changed states; the game ignores duplicate states and release-only
changes. For independent buttons/drums, only new `0 -> 1` bits are scored.
Guitar, bass, and Custom 1 use section values: a changed nonzero fret value is
scored in full, while unchanged strings are ignored. Repeated drum strikes
must include a release state between presses, even if both packets arrive
within one game frame. Each update reads four bytes and processes the input
only when `readBytes()` returns exactly four. Short reads are discarded and
never combined with a later read. Custom 3 Lua functions receive the new press bits.

## Achievements

The song-end menu shows each player's hit count and loaded-note count, accuracy,
and the medals earned by the band. Newly unlocked medals are marked **NEW**;
hover over a medal to see its requirement. Unlocks are shared locally and saved
in `medals.bin`, relative to the game's working directory. The Achievements
menu shows saved unlocks and dims locked medals.

- **First Gig:** finish your first nonempty, fully loaded chart, including
  notes that were missed. Empty, missing, truncated, and unfinished charts do
  not earn song medals.
- **Completion:** hit 100% of the notes in a completed chart. Incorrect extra
  presses do not prevent this medal.
- **Bass / Drums / Guitar and Easy / Hard:** finish a chart on that instrument
  or difficulty. Custom instruments can earn the general and difficulty medals.
- **Full Combo:** hit every chart note without any incorrect new presses.
  Releases, unchanged states, and valid partial chords do not break the combo.
  A chord counts once after all its required inputs are played.
- **Instrument Mastery:** earn a Full Combo on Hard with the named instrument.
- **Streak:** hit 100 consecutive notes; a missed note or incorrect press
  resets the streak.
- **Band Together / Four Player:** every player finishes their chart in a band
  of at least two / exactly four players.
- **Setlist:** every player finishes every chart in a setlist of at least two
  songs. Leaving early does not earn this medal.

LAN, Online, and Medium medals are reserved for modes not yet implemented.

Build and run the achievement tests with:

```sh
cmake -S . -B build -DBANDHERO_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For menu screenshots in a graphical session, run `AchievementsTests --render`
from `build/achievement-tests/` (the executable is in its parent directory).
The harness uses isolated fixture folders and saves, leaving player data intact.

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

