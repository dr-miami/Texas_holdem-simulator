# Texas Hold'em Poker (C++)

A terminal-based Texas Hold'em Poker game written in C++17. Features configurable CPU opponents, automated hand evaluation, cross-platform terminal controls, and background audio integration via `miniaudio`.

---

## Features

* **Multi-CPU Opponents:** Play against 1 to 4 CPU opponents with dynamic betting decision logic.
* **Audio Integration:** Background music and custom sound effects for winning and losing hands powered by `miniaudio`.
* **Deck Scaling:** Automatically adjusts the deck starting rank based on the number of players to ensure competitive play.
* **Base-100 Hand Evaluator:** Accurately evaluates and scores poker hands to guarantee tie-breakers and proper winner determination.
* **Cross-Platform:** Terminal-safe screen clearing and non-blocking key inputs compatible with Linux and Windows.

---

## Game & Sound Assets

Place the following sound and text files in the root folder alongside your source code:

* `sound.wav` — Background music (loops during gameplay)
* `win.wav` — Winning hand sound effect
* `loss.wav` — Losing hand / Fold sound effect
* `rules.txt` — Game rules displayed from the main menu
* `points.txt` — Hand rankings and scoring breakdown displayed from the main menu

---

## Requirements & Dependencies

### Linux Systems
Install the required audio development headers before compiling:

```bash
# Ubuntu / Debian / Pop!_OS
sudo apt update && sudo apt install libasound2-dev libpulse-dev

# Arch Linux
sudo pacman -S alsa-lib libpulse

# Fedora
sudo dnf install alsa-lib-devel pulseaudio-libs-devel

```

---

## Building & Running

### Compilation

**Linux (GCC / Clang):**

```bash
g++ -std=c++17 main.cpp -lpthread -ldl -lm -lasound -lpulse -o Poker

```

**Windows (MinGW):**

```bash
g++ -std=c++17 main.cpp -o Poker.exe

```

### Running the Game

Always launch the binary from the directory where the source code and asset files are located:

```bash
# Linux
./Poker

# Windows
Poker.exe

```

---

## How to Play

1. Start with an initial balance of **$1000**.
2. Select the number of **CPU opponents (1–4)** at startup.
3. Use the arrow keys or `W`/`S` keys (and `ENTER`) to navigate the main menu.
4. Each round automatically posts small and big blinds before dealing hole cards.
5. On your turn during betting rounds (**Pre-Flop**, **Flop**, **Turn**, **River**), choose:
* **`C`** to Check or Call
* **`R`** to Raise (up to a max of 3 raises per round)
* **`F`** to Fold


6. Reaching the **Showdown** evaluates active hands and awards the pot to the best hand.

---

## Project Structure

```
.
├── main.cpp        # Game execution loop, audio initialization, user interface
├── functions.hpp   # Card definitions, deck shuffling, CPU AI logic, hand evaluation
├── miniaudio.h     # Single-header cross-platform audio library
├── sound.wav       # Background music track
├── win.wav         # Victory sound effect
├── loss.wav        # Defeat sound effect
├── rules.txt       # Rules text file
└── points.txt      # Hand rankings text file

```

```