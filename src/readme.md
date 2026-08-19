# 🃏 Terminal Texas Hold'em

A high-performance, interactive Texas Hold'em poker game built for the Windows Terminal. This project features a custom hand-evaluation engine, dynamic UI updates, and integrated audio support.

![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Language](https://img.shields.io/badge/Language-C++-blue.svg)

---

## ✨ Features

*   **Arrow-Key Navigation:** Smooth menu system using standard keyboard arrows and Enter.
*   **Dynamic UI Engine:** A custom "Refresh UI" system that tracks the Pot, Balance, and Table states in real-time.
*   **Audio Integration:** Uses the `miniaudio` library to play `sound.wav` during the session.
*   **Authentic Gameplay:** Includes the standard Poker phases: **The Flop**, **The Turn**, and **The River**.
*   **Sophisticated Hand Evaluation:** 
    *   Automatically detects your best hand from hole cards and community cards.
    *   Calculates payouts based on hand-strength multipliers.
*   **Showdown Mode:** Reveals CPU cards and compares scores to determine the winner.

---

## 🛠 Prerequisites & Setup

### Requirements
*   **OS:** Windows (uses `<windows.h>` and `<conio.h>`)
*   **Compiler:** G++ / MinGW or MSVC
*   **Audio File:** A `sound.wav` file must be present in the root directory.
*   **Assets:** A `.files/` folder containing `rules.txt` and `points.txt`.

### Compilation
To compile via the command line using MinGW:

```bash
g++ main.cpp -o PokerGame.exe -lwinmm
