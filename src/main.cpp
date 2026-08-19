#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "functions.hpp"
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

// Global audio helper using pre-allocated single-engine audio nodes
void playOneShot(ma_sound* pSound, ma_sound* pBgm, int durationMs) {
    if (pBgm) ma_sound_stop(pBgm);
    if (pSound) {
        ma_sound_seek_to_pcm_frame(pSound, 0);
        ma_sound_start(pSound);
    }
    sleepMs(durationMs);
    if (pSound) ma_sound_stop(pSound);
    if (pBgm) ma_sound_start(pBgm);
}

bool bettingRound(std::vector<Card>& pHand,
                  std::vector<std::vector<Card>>& cpuHands,
                  const std::vector<Card>& table,
                  std::string phase,
                  std::vector<bool>& cpuActive,
                  ma_sound* pWinSfx,
                  ma_sound* pLossSfx,
                  ma_sound* pBgm) {
    int betToCall  = 0;
    int raiseCount = 0;
    bool playerActive = true;
    bool firstTime    = true;
    int  lastRaiser   = -1;

    while (firstTime || lastRaiser != -1) {
        firstTime = false;

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
                playOneShot(pLossSfx, pBgm, 3000);
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

        bool anyActiveCpu = false;
        for (int i = 0; i < numCPUs; ++i) {
            if (!cpuActive[i]) continue;
            anyActiveCpu = true;
            if (lastRaiser == i + 1) continue;

            refreshUI(pHand, table, phase + " - CPU " + std::to_string(i + 1) + " TURN",
                      betToCall, raiseCount);
            std::cout << "\nCPU " << (i + 1) << " is thinking..." << std::flush;
            sleepRandom();

            Decision d = cpuDecideBet(cpuHands[i], table, betToCall, raiseCount, MAX_RAISES);

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

        if (!anyActiveCpu && playerActive) {
            bool allFolded = true;
            for (int i = 0; i < numCPUs; ++i)
                if (cpuActive[i]) { allFolded = false; break; }
            if (allFolded) {
                std::cout << GREEN << "All CPUs folded. You win $" << pot << "!" << RESET << "\n";
                playerBalance += pot;
                pot = 0;
                sleepMs(50);
                playOneShot(pWinSfx, pBgm, 4000);
                return false;
            }
        }

        if (lastRaiser == 0) {
            bool allResponded = true;
            for (int i = 0; i < numCPUs; ++i)
                if (cpuActive[i]) { allResponded = false; break; }
            if (allResponded) break;
            lastRaiser = -1;
        }

        if (lastRaiser == -1) break;
    }
    return true;
}

void playGame(ma_sound* pWinSfx, ma_sound* pLossSfx, ma_sound* pBgm) {
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

    std::vector<std::vector<Card>> cpuHands(numCPUs);
    std::vector<bool> cpuActive(numCPUs, true);
    for (int i = 0; i < numCPUs; ++i)
        cpuHands[i] = {myDeck.drawCard(), myDeck.drawCard()};

    std::vector<Card> tableCards;

    // Pre-flop
    if (!bettingRound(playerHand, cpuHands, tableCards, "PRE-FLOP", cpuActive, pWinSfx, pLossSfx, pBgm)) return;

    // FLOP
    for (int i = 0; i < 3 && !myDeck.isEmpty(); ++i) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the FLOP..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "FLOP", cpuActive, pWinSfx, pLossSfx, pBgm)) return;

    // TURN
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the TURN..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "TURN", cpuActive, pWinSfx, pLossSfx, pBgm)) return;

    // RIVER
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the RIVER..." << std::endl;
    sleepMs(1500);
    if (!bettingRound(playerHand, cpuHands, tableCards, "RIVER", cpuActive, pWinSfx, pLossSfx, pBgm)) return;

    // SHOWDOWN
    clearScreen();
    printHeader();
    std::cout << RED << BOLD << "\n*** SHOWDOWN ***" << RESET << "\n\n";

    HandResult playerResult = evaluateHand(playerHand, tableCards);

    std::cout << "YOUR HAND: ";
    for (const auto& c : playerHand) c.print();
    std::cout << " (" << playerResult.name << ")\n";

    int overallWinner = 0; 
    HandResult bestCpuResult = playerResult; 
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
            overallWinner = 1; 
        } else if (cpuResult.totalScore == playerResult.totalScore && overallWinner == 0) {
            overallWinner = -1; 
        }
    }
    std::cout << "\n";

    if (overallWinner == 0) {
        playerBalance += pot;
        std::cout << GREEN << BOLD << ">> YOU WIN $" << pot << " with " << playerResult.name << "! <<" << RESET << "\n";
        playOneShot(pWinSfx, pBgm, 4000);
    } else if (overallWinner == 1) {
        std::cout << RED << BOLD << ">> CPU " << (bestCpuIndex + 1) << " WINS $" << pot
                  << " with " << bestCpuResult.name << "! <<" << RESET << "\n";
        playOneShot(pLossSfx, pBgm, 4000);
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

    ma_engine engine;
    ma_sound bgSound, winSound, lossSound;
    bool audioReady = false;

    // Single global audio engine initialization
    if (ma_engine_init(NULL, &engine) == MA_SUCCESS) {
        bool bgmOk  = (ma_sound_init_from_file(&engine, "sound.wav", MA_SOUND_FLAG_STREAM, NULL, NULL, &bgSound) == MA_SUCCESS);
        bool winOk  = (ma_sound_init_from_file(&engine, "win.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &winSound) == MA_SUCCESS);
        bool lossOk = (ma_sound_init_from_file(&engine, "loss.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &lossSound) == MA_SUCCESS);

        if (bgmOk && winOk && lossOk) {
            ma_sound_set_looping(&bgSound, true);
            ma_sound_start(&bgSound);
            audioReady = true;
        }
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
        if (key == 224 || key == 27) {
            if (key == 27) { _getch(); }
            key = _getch();
            if (key == 72 || key == 'A') choice = (choice - 1 + 4) % 4;
            if (key == 80 || key == 'B') choice = (choice + 1) % 4;
        } else if (key == 13 || key == 10) {
            if      (choice == 0) playGame(audioReady ? &winSound : nullptr, audioReady ? &lossSound : nullptr, audioReady ? &bgSound : nullptr);
            else if (choice == 1) showFile("rules.txt");
            else if (choice == 2) showFile("points.txt");
            else if (choice == 3) break;
        }
    }

    if (audioReady) {
        ma_sound_uninit(&bgSound);
        ma_sound_uninit(&winSound);
        ma_sound_uninit(&lossSound);
        ma_engine_uninit(&engine);
    }
    return 0;
}