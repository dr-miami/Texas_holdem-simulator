#include "functions.hpp"
#include <windows.h>
#include "miniaudio.h"
#include <stdio.h>

void refreshUI(const std::vector<Card>& pHand, const std::vector<Card>& table,
               std::string phase, int toCall = 0, int raiseCount = 0) {
    clearScreen();
    printHeader();
    std::cout << YELLOW << "Pot: $" << pot << " | Your Balance: $" << playerBalance
              << " | To Call: $" << toCall << RESET << "\n";
    if (raiseCount >= MAX_RAISES) {
        std::cout << RED << "BETTING CAPPED (Max 3 raises)" << RESET << "\n";
    }
    std::cout << GREEN << BOLD << "\n--- " << phase << " ---" << RESET << "\n\n";
    std::cout << BOLD << "YOUR HAND: " << RESET;
    for (const auto& c : pHand) c.print();
    std::cout << "\n" << YELLOW << BOLD << "CPU HAND(S): " << RESET;
    for (int i = 0; i < numCPUs; ++i)
        std::cout << "CPU " << (i + 1) << ": [??][??]  ";
    std::cout << "(Hidden)\n\n";
    std::cout << BOLD << "TABLE:     " << RESET;
    if (table.empty()) std::cout << "(Waiting for cards...)";
    else for (const auto& c : table) c.print();
    std::cout << "\n----------------------------------------\n";
}

// Helper: stop background music, play a one-shot sound, then restart background music.
void playOneShot(ma_engine* pMainEngine, const char* file) {
    ma_engine_stop(pMainEngine);
    ma_engine eng;
    ma_engine_init(NULL, &eng);
    ma_engine_play_sound(&eng, file, NULL);
    sleepMs(4000);
    ma_engine_uninit(&eng);
    ma_engine_start(pMainEngine);
}

// Returns true if the round continues (no one folded to end it early),
// false if the player folded or all CPUs folded (round is over).
bool bettingRound(std::vector<Card>& pHand,
                  std::vector<std::vector<Card>>& cpuHands,
                  const std::vector<Card>& table,
                  std::string phase,
                  std::vector<bool>& cpuActive,
                  ma_engine* pMainEngine) {
    int betToCall  = 0;
    int raiseCount = 0;
    bool playerActive = true;
    bool firstTime    = true;
    int  lastRaiser   = -1; // -1=none, 0=player, 1..N=cpu index+1

    while (firstTime || lastRaiser != -1) {
        firstTime = false;

        // --- Player Turn ---
        if (playerActive && lastRaiser != 0) {
            refreshUI(pHand, table, phase + " - YOUR TURN", betToCall, raiseCount);

            if (betToCall == 0) {
                if (raiseCount < MAX_RAISES)
                    std::cout << "[C]heck  [R]aise  [F]old\n> ";
                else
                    std::cout << "[C]heck  [F]old (betting capped)\n> ";
            } else {
                if (raiseCount < MAX_RAISES)
                    std::cout << "[C]all $" << betToCall << "  [R]aise  [F]old\n> ";
                else
                    std::cout << "[C]all $" << betToCall << "  [F]old (betting capped)\n> ";
            }

            char cmd;
            std::cin >> cmd;
            std::cin.ignore(1000, '\n');
            cmd = toupper(cmd);

            if (cmd == 'F') {
                std::cout << RED << "You fold." << RESET << "\n";
                playerActive = false;
                sleepMs(1000);
                playOneShot(pMainEngine, "loss.wav");
                return false;
            } else if (cmd == 'C') {
                if (betToCall > 0) {
                    std::cout << GREEN << "You call $" << betToCall << "." << RESET << "\n";
                    playerBalance -= betToCall;
                    pot += betToCall;
                } else {
                    std::cout << GREEN << "You check." << RESET << "\n";
                }
                sleepMs(800);
                // If a CPU was the last to raise and player just called, end round
                if (lastRaiser > 0) { lastRaiser = -1; break; }
            } else if (cmd == 'R') {
                if (raiseCount >= MAX_RAISES) {
                    std::cout << RED << "Max raises reached! Calling instead..." << RESET << "\n";
                    if (betToCall > 0) { playerBalance -= betToCall; pot += betToCall; }
                    sleepMs(1000);
                } else {
                    int minRaise = (betToCall == 0) ? 20 : betToCall * 2;
                    std::cout << "Enter raise amount (min $" << minRaise << "): $";
                    int newBet;
                    std::cin >> newBet;
                    std::cin.ignore(1000, '\n');
                    if (newBet >= minRaise && newBet <= playerBalance) {
                        pot += newBet;
                        playerBalance -= newBet;
                        betToCall = newBet;
                        raiseCount++;
                        lastRaiser = 0;
                        std::cout << YELLOW << "You raise to $" << betToCall << "!" << RESET << "\n";
                    } else {
                        std::cout << "Invalid. Calling instead.\n";
                        if (betToCall > 0) { playerBalance -= betToCall; pot += betToCall; }
                    }
                    sleepMs(800);
                }
            }
        }

        // --- CPU Turns ---
        bool anyActiveCpu = false;
        for (int i = 0; i < numCPUs; ++i) {
            if (!cpuActive[i]) continue;
            anyActiveCpu = true;
            // Skip the CPU that just raised (don't let it act again in the same pass)
            if (lastRaiser == i + 1) continue;

            refreshUI(pHand, table, phase + " - CPU " + std::to_string(i + 1) + " TURN",
                      betToCall, raiseCount);
            std::cout << "\nCPU " << (i + 1) << " is thinking..." << std::flush;
            sleepRandom();

            Decision d = cpuDecideBet(cpuHands[i], table, betToCall, raiseCount, MAX_RAISES);

            // Enforce raise cap
            if (d.action == BetAction::RAISE && raiseCount >= MAX_RAISES)
                d.action = BetAction::CALL;

            switch (d.action) {
                case BetAction::FOLD:
                    std::cout << RED << " CPU " << (i + 1) << " folds!" << RESET << "\n";
                    cpuActive[i] = false;
                    sleepMs(1000);
                    break;
                case BetAction::CHECK:
                    std::cout << CYAN << " CPU " << (i + 1) << " checks." << RESET << "\n";
                    sleepMs(1000);
                    break;
                case BetAction::CALL:
                    std::cout << CYAN << " CPU " << (i + 1) << " calls $" << betToCall << "." << RESET << "\n";
                    pot += betToCall;
                    sleepMs(1000);
                    break;
                case BetAction::RAISE:
                    raiseCount++;
                    betToCall  = d.amount;
                    pot       += betToCall;
                    lastRaiser = i + 1;
                    std::cout << YELLOW << " CPU " << (i + 1) << " raises to $" << betToCall << "!" << RESET << "\n";
                    sleepMs(1000);
                    break;
            }
        }

        // If all CPUs folded, player wins the pot
        if (!anyActiveCpu && playerActive) {
            // Count remaining active CPUs after this pass
            bool allFolded = true;
            for (int i = 0; i < numCPUs; ++i)
                if (cpuActive[i]) { allFolded = false; break; }
            if (allFolded) {
                std::cout << GREEN << "All CPUs folded. You win $" << pot << "!" << RESET << "\n";
                playerBalance += pot;
                pot = 0;
                sleepMs(50);
                playOneShot(pMainEngine, "win.wav");
                return false;
            }
        }

        // If player raised and all CPUs have now responded, round ends
        if (lastRaiser == 0) {
            bool allResponded = true;
            for (int i = 0; i < numCPUs; ++i)
                if (cpuActive[i]) { allResponded = false; break; }
            if (allResponded) break;
            // else keep going so CPUs can call/raise
            lastRaiser = -1;
        }

        // If no one raised this full pass, the round is over
        if (lastRaiser == -1) break;
    }
    return true;
}

void playGame(ma_engine* pMainEngine) {
    clearScreen();
    printHeader();
    srand(time(nullptr));

    pot = 0;

    std::cout << "Small blind $10, Big blind $20\n";
    playerBalance -= 20;
    pot += 30;
    currentBet = 20;
    sleepMs(1000);

    Deck myDeck(startingRank);
    myDeck.shuffleDeck();

    std::vector<Card> playerHand = {myDeck.drawCard(), myDeck.drawCard()};

    // Deal one hand per CPU
    std::vector<std::vector<Card>> cpuHands(numCPUs);
    std::vector<bool> cpuActive(numCPUs, true);
    for (int i = 0; i < numCPUs; ++i)
        cpuHands[i] = {myDeck.drawCard(), myDeck.drawCard()};

    std::vector<Card> tableCards;

    // Pre-flop
    if (!bettingRound(playerHand, cpuHands, tableCards, "PRE-FLOP", cpuActive, pMainEngine)) return;

    // FLOP
    for (int i = 0; i < 3 && !myDeck.isEmpty(); ++i) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the FLOP..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "FLOP", cpuActive, pMainEngine)) return;

    // TURN
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the TURN..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "TURN", cpuActive, pMainEngine)) return;

    // RIVER
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the RIVER..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "RIVER", cpuActive, pMainEngine)) return;

    // --- SHOWDOWN ---
    clearScreen();
    printHeader();
    std::cout << RED << BOLD << "\n*** SHOWDOWN ***" << RESET << "\n\n";

    HandResult playerResult = evaluateHand(playerHand, tableCards);

    std::cout << "YOUR HAND: ";
    for (const auto& c : playerHand) c.print();
    std::cout << " (" << playerResult.name << ")\n";

    // Evaluate each active CPU
    int overallWinner = 0; // 0 = player wins by default
    HandResult bestCpuResult = playerResult; // start benchmark at player score
    int bestCpuIndex = -1;

    for (int i = 0; i < numCPUs; ++i) {
        HandResult cpuResult = evaluateHand(cpuHands[i], tableCards);
        std::cout << YELLOW << "CPU " << (i + 1) << " HAND: ";
        for (const auto& c : cpuHands[i]) c.print();
        std::cout << RESET;
        if (!cpuActive[i]) {
            std::cout << " (folded)\n";
            continue;
        }
        std::cout << " (" << cpuResult.name << ")\n";

        if (cpuResult.totalScore > bestCpuResult.totalScore) {
            bestCpuResult = cpuResult;
            bestCpuIndex  = i;
            overallWinner = 1; // a CPU is currently winning
        } else if (cpuResult.totalScore == playerResult.totalScore && overallWinner == 0) {
            overallWinner = -1; // draw with player (so far)
        }
    }
    std::cout << "\n";

    // Determine final outcome
    // overallWinner: 0=player, 1=a CPU won, -1=draw
    if (overallWinner == 0) {
        playerBalance += pot;
        std::cout << GREEN << BOLD << ">> YOU WIN $" << pot << " with " << playerResult.name << "! <<" << RESET << "\n";

        ma_engine_stop(pMainEngine);
        ma_engine eng;
        ma_engine_init(NULL, &eng);
        ma_engine_play_sound(&eng, "win.wav", NULL);
        sleepMs(5000);
        ma_engine_uninit(&eng);
        ma_engine_start(pMainEngine);
    } else if (overallWinner == 1) {
        std::cout << RED << BOLD << ">> CPU " << (bestCpuIndex + 1) << " WINS $" << pot
                  << " with " << bestCpuResult.name << "! <<" << RESET << "\n";

        ma_engine_stop(pMainEngine);
        ma_engine eng;
        ma_engine_init(NULL, &eng);
        ma_engine_play_sound(&eng, "loss.wav", NULL);
        sleepMs(5000);
        ma_engine_uninit(&eng);
        ma_engine_start(pMainEngine);
    } else {
        playerBalance += pot / 2;
        std::cout << YELLOW << BOLD << ">> DRAW! Pot split. You receive $" << pot / 2 << "." << RESET << std::endl;
    }

    pot = 0;
    std::cout << BOLD << "\nNEW BALANCE: $" << playerBalance << RESET << std::endl;
    waitForEnter();
}

int main() {
    bootingSequence();

    ma_engine engine1;
    if (ma_engine_init(NULL, &engine1) != MA_SUCCESS) return -1;

    ma_sound bgSound;
    if (ma_sound_init_from_file(&engine1, "sound.wav", MA_SOUND_FLAG_STREAM, NULL, NULL, &bgSound) == MA_SUCCESS) {
        ma_sound_set_looping(&bgSound, true);
        ma_sound_start(&bgSound);
    }

    chooseOpponents();

    int choice = 0;
    std::string options[] = {"Start Game", "Rules", "Hand Rankings", "Quit"};

    while (true) {
        clearScreen();
        printHeader();
        std::cout << YELLOW << "BALANCE: $" << playerBalance << RESET << "\n";
        std::cout << CYAN  << "Opponents: " << numCPUs << " CPU(s) | Deck from rank " << startingRank << RESET << "\n\n";
        for (int i = 0; i < 4; i++) {
            if (i == choice) std::cout << GREEN << BOLD << "  > " << options[i] << " <" << RESET << std::endl;
            else             std::cout << "    " << options[i] << std::endl;
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            if (key == 72) choice = (choice - 1 + 4) % 4;
            if (key == 80) choice = (choice + 1) % 4;
        } else if (key == 13) {
            if      (choice == 0) playGame(&engine1);
            else if (choice == 1) showFile(".files\\rules.txt");
            else if (choice == 2) showFile(".files\\points.txt");
            else if (choice == 3) break;
        }
    }

    ma_sound_stop(&bgSound);
    ma_sound_uninit(&bgSound);
    ma_engine_uninit(&engine1);
    return 0;
}