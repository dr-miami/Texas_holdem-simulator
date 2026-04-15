🃏 Terminal Texas Hold'em
A feature-rich, command-line interface (CLI) Texas Hold'em poker game written in C++. This project features real-time UI updates, sound effects via the miniaudio library, and a full hand-evaluation engine.

Language Platform

🚀 Features
Interactive Menu: Navigate through options using arrow keys.
Dynamic UI: Colorful terminal output using ANSI escape codes for a modern look.
Full Game Logic: Includes betting, card shuffling, dealing, and phases (Flop, Turn, River).
Hand Evaluation: Automatically calculates the winner and determines hand rankings (e.g., Full House, Flush, etc.).
Audio Support: Integrated sound effects using the miniaudio engine.
Persistence: View game rules and hand rankings directly from external text files.
🛠 Prerequisites
This project is designed for Windows environments due to the use of <windows.h> and _getch().

Compiler: A C++ compiler (like MinGW/GCC or MSVC).
Audio Library: miniaudio.h (single-header library).
Sound Assets: A file named sound.wav in the root directory.
📂 Project Structure
├── main.cpp             # Main game loop and UI logic
├── functions.hpp        # Core game logic, Card/Deck classes, and hand evaluation
├── miniaudio.h          # Audio engine header
├── sound.wav            # Sound effect file
└── .files/
    ├── rules.txt        # Game instructions
    └── points.txt       # Hand ranking information
🎮 How to Play
Launch the Game: Run the compiled executable.
Main Menu: Use the Up/Down Arrow Keys to navigate and Enter to select.
Place Your Bet: Enter the amount you wish to wager against the CPU.
Gameplay:
View your hand and the current pot.
Press any key to progress through the Flop, Turn, and River.
Showdown: The CPU reveals its hand, and the winner is determined based on standard poker rankings.
Payouts: Your balance updates automatically based on the hand multiplier if you win.
🛠 Compilation
To compile the project using G++, run the following command in your terminal:

g++ main.cpp -o PokerGame.exe -lwinmm
Note: The -lwinmm flag is required for audio playback on Windows.

🖥 Code Snippet: Hand Evaluation
The game utilizes a scoring system to compare player and CPU hands:

HandResult playerResult = evaluateHand(playerHand, tableCards);
HandResult cpuResult = evaluateHand(cpuHand, tableCards);

if (playerResult.totalScore > cpuResult.totalScore) {
    // Logic for Player Victory
}
📝 License
This project is open-source. Feel free to modify and expand upon the poker engine!

