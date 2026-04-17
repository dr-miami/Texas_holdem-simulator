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
    std::cout << "\n" << YELLOW << BOLD << "CPU HAND:  " << RESET << "[??] [??] (Hidden)\n\n";
    std::cout << BOLD << "TABLE:     " << RESET;
    if (table.empty()) std::cout << "(Waiting for cards...)";
    else for (const auto& c : table) c.print();
    std::cout << "\n----------------------------------------\n";
}

bool bettingRound(std::vector<Card>& pHand, std::vector<Card>& cpuHand, 
                  const std::vector<Card>& table, std::string phase) {
    int betToCall = 0;
    int raiseCount = 0;
    bool playerActive = true;
    bool cpuActive = true;
    bool firstTime = true;
    int lastRaiser = -1; // -1 = no one, 0 = player, 1 = CPU
    
    while (firstTime || lastRaiser != -1) {
        firstTime = false;
        
        // Check if betting is done
        if (lastRaiser == 0 && !firstTime) break; // Back to player after CPU called
        if (lastRaiser == 1) { // CPU raised, player must respond
            // Player's turn forced
        }
        
        // Player Turn
        if (playerActive && (lastRaiser != 0 || betToCall > 0)) {
            refreshUI(pHand, table, phase + " - YOUR TURN", betToCall, raiseCount);
            
            if (betToCall == 0) {
                if (raiseCount < MAX_RAISES) {
                    std::cout << "[C]heck  [R]aise  [F]old\n> ";
                } else {
                    std::cout << "[C]heck  [F]old (betting capped)\n> ";
                }
            } else {
                if (raiseCount < MAX_RAISES) {
                    std::cout << "[C]all $" << betToCall << "  [R]aise  [F]old\n> ";
                } else {
                    std::cout << "[C]all $" << betToCall << "  [F]old (betting capped)\n> ";
                }
            }
            
            char cmd;
            std::cin >> cmd;
            std::cin.ignore(1000, '\n');
            cmd = toupper(cmd);
            
            if (cmd == 'F') {
                std::cout << RED << "You fold." << RESET << "\n";
                playerActive = false;
                sleepMs(1000);
                return false; // Hand ends
            }
            else if (cmd == 'C') {
                if (betToCall > 0) {
                    std::cout << GREEN << "You call $" << betToCall << "." << RESET << "\n";
                    playerBalance -= betToCall;
                    pot += betToCall;
                } else {
                    std::cout << GREEN << "You check." << RESET << "\n";
                }
                sleepMs(800);
                
                if (lastRaiser == 1) break; // Called CPU's raise, betting round done
            }
            else if (cmd == 'R') {
                if (raiseCount >= MAX_RAISES) {
                    std::cout << RED << "Max raises reached! Calling instead..." << RESET << "\n";
                    if (betToCall > 0) {
                        playerBalance -= betToCall;
                        pot += betToCall;
                    }
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
                        if (betToCall > 0) {
                            playerBalance -= betToCall;
                            pot += betToCall;
                        }
                    }
                    sleepMs(800);
                }
            }
        }
        
        if (!cpuActive) break;
        
        // CPU Turn
        refreshUI(pHand, table, phase + " - CPU TURN", betToCall, raiseCount);
        std::cout << "\nCPU is thinking..." << std::flush;
        sleepRandom(); // 1-2 second delay
        
        Decision d = cpuDecideBet(cpuHand, table, betToCall, raiseCount, MAX_RAISES);
        
        // Enforce raise cap on CPU
        if (d.action == BetAction::RAISE && raiseCount >= MAX_RAISES) {
            d.action = BetAction::CALL;
        }
        
        switch (d.action) {
            case BetAction::FOLD:
                std::cout << RED << " CPU folds!" << RESET << "\n";
                cpuActive = false;
                sleepMs(1000);
                if (playerActive) {
                    // Player wins the pot
                    std::cout << GREEN << "CPU folded. You win $" << pot << "!" << RESET << "\n";
                    playerBalance += pot;
                    pot = 0;
                    sleepMs(2000);
                    return false; // Hand ends
                }
                break;
            case BetAction::CHECK:
                std::cout << CYAN << " CPU checks." << RESET << "\n";
                if (lastRaiser == 0 && betToCall == 0) { // Both checked
                    sleepMs(1000);
                    return true;
                }
                break;
            case BetAction::CALL:
                std::cout << CYAN << " CPU calls $" << betToCall << "." << RESET << "\n";
                pot += betToCall;
                sleepMs(1000);
                if (lastRaiser == 0) { // Called player's raise
                    return true;
                }
                break;
            case BetAction::RAISE:
                raiseCount++;
                betToCall = d.amount;
                pot += betToCall;
                lastRaiser = 1;
                std::cout << YELLOW << " CPU raises to $" << betToCall << "!" << RESET << "\n";
                sleepMs(1000);
                break;
        }
    }
    return true;
}

void playGame(ma_engine* pMainEngine) {
    clearScreen();
    printHeader();
    srand(time(nullptr));
    
    // Reset pot
    pot = 0;
    
    // Blinds/Ante
    std::cout << "Small blind $10, Big blind $20\n";
    playerBalance -= 20;
    pot += 30; // Player posts big blind, CPU posts small blind + match
    currentBet = 20;
    sleepMs(1000);
    
    Deck myDeck;
    myDeck.shuffleDeck();
    
    // Deal hole cards
    std::vector<Card> playerHand = {myDeck.drawCard(), myDeck.drawCard()};
    std::vector<Card> cpuHand = {myDeck.drawCard(), myDeck.drawCard()};
    std::vector<Card> tableCards;

    // Pre-flop betting
    if (!bettingRound(playerHand, cpuHand, tableCards, "PRE-FLOP")) return;

    // FLOP (3 cards)
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    
    std::cout << "\nDealing the FLOP..." << std::endl;
    sleepMs(1500);
    
    if (!bettingRound(playerHand, cpuHand, tableCards, "FLOP")) return;

    // TURN (4th card)
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the TURN..." << std::endl;
    sleepMs(1500);
    
    if (!bettingRound(playerHand, cpuHand, tableCards, "TURN")) return;

    // RIVER (5th card)
    if (!myDeck.isEmpty()) tableCards.push_back(myDeck.drawCard());
    std::cout << "\nDealing the RIVER..." << std::endl;
    sleepMs(1500);
    
    if (!bettingRound(playerHand, cpuHand, tableCards, "RIVER")) return;

    // SHOWDOWN
    clearScreen();
    printHeader();
    std::cout << RED << BOLD << "\n*** SHOWDOWN ***" << RESET << "\n\n";
    
    HandResult playerResult = evaluateHand(playerHand, tableCards);
    HandResult cpuResult = evaluateHand(cpuHand, tableCards);

    std::cout << "YOUR HAND: "; 
    for (const auto& c : playerHand) c.print();
    std::cout << " (" << playerResult.name << ")\n";
    
    std::cout << YELLOW << "CPU HAND:  "; 
    for (const auto& c : cpuHand) c.print();
    std::cout << RESET << " (" << cpuResult.name << ")\n\n";

    if (playerResult.totalScore > cpuResult.totalScore) {
        playerBalance += pot;
        std::cout << GREEN << BOLD << ">> YOU WIN $" << pot << " with " << playerResult.name << "! <<" << RESET << "\n";
        
        // Win sound
        ma_engine_stop(pMainEngine);
        ma_engine eng; 
        ma_engine_init(NULL, &eng);
        ma_engine_play_sound(&eng, "jackpot.wav", NULL);
        sleepMs(3000);
        ma_engine_uninit(&eng);
        ma_engine_start(pMainEngine);
    } 
    else if (cpuResult.totalScore > playerResult.totalScore) {
        std::cout << RED << BOLD << ">> CPU WINS $" << pot << " with " << cpuResult.name << "! <<" << RESET << "\n";
        
        // Lose sound
        ma_engine_stop(pMainEngine);
        ma_engine eng; 
        ma_engine_init(NULL, &eng);
        ma_engine_play_sound(&eng, "fart.wav", NULL);
        sleepMs(2000);
        ma_engine_uninit(&eng);
        ma_engine_start(pMainEngine);
    } 
    else {
        playerBalance += pot / 2; // Split pot
        std::cout << YELLOW << BOLD << ">> DRAW! Pot split. <<" << RESET << std::endl;
        sleepMs(2000);
    }
    
    pot = 0;
    std::cout << BOLD << "\nNEW BALANCE: $" << playerBalance << RESET << std::endl;
    waitForEnter();
}

int main() {
    bootingSequence();
    
    ma_engine engine1;
    if (ma_engine_init(NULL, &engine1) != MA_SUCCESS) {
        return -1;
    }

    // Setup looping background music
    ma_sound bgSound;
    if (ma_sound_init_from_file(&engine1, "sound.wav", MA_SOUND_FLAG_STREAM, NULL, NULL, &bgSound) == MA_SUCCESS) {
        ma_sound_set_looping(&bgSound, true);
        ma_sound_start(&bgSound);
    }

    int choice = 0;
    std::string options[] = {"Start Game", "Rules", "Hand Rankings", "Quit"};

    while (true) {
        clearScreen();
        printHeader();
        std::cout << YELLOW << "BALANCE: $" << playerBalance << RESET << "\n\n";
        for (int i = 0; i < 4; i++) {
            if (i == choice) std::cout << GREEN << BOLD << "  > " << options[i] << " <" << RESET << std::endl;
            else std::cout << "    " << options[i] << std::endl;
        }

        int key = _getch();
        if (key == 224) { 
            key = _getch();
            if (key == 72) choice = (choice - 1 + 4) % 4; 
            if (key == 80) choice = (choice + 1) % 4;               
        } else if (key == 13) { 
            if (choice == 0) {
                playGame(&engine1);
            }
            else if (choice == 1) showFile(".files\\rules.txt");
            else if (choice == 2) showFile(".files\\points.txt");
            else if (choice == 3) break;
        }
    }

    // Cleanup
    ma_sound_stop(&bgSound);
    ma_sound_uninit(&bgSound);
    ma_engine_uninit(&engine1);
    return 0;
}
