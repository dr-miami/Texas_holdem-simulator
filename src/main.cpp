#include "functions.hpp"
#include <windows.h>

#include "miniaudio.h"

#include <stdio.h>

void refreshUI(const std::vector<Card>& pHand, const std::vector<Card>& table, std::string phase) {
    clearScreen();
    printHeader();
    std::cout << YELLOW << "Pot: $" << currentBet << " | Your Balance: $" << playerBalance - currentBet << RESET << "\n";
    std::cout << GREEN << BOLD << "\n--- " << phase << " ---" << RESET << "\n\n";
    std::cout << BOLD << "YOUR HAND: " << RESET;
    for (const auto& c : pHand) c.print();
    std::cout << "\n" << BOLD << "CPU HAND:  " << RESET << "[??] [??] (Hidden)\n\n";
    std::cout << BOLD << "TABLE:     " << RESET;
    if (table.empty()) std::cout << "(Waiting for cards...)";
    else for (const auto& c : table) c.print();
    std::cout << "\n----------------------------------------\n";
}

void playGame() {
    clearScreen();
    printHeader();
    placeBet(); 

    Deck myDeck;
    myDeck.shuffleDeck();
    std::vector<Card> playerHand = {myDeck.drawCard(), myDeck.drawCard()};
    std::vector<Card> cpuHand    = {myDeck.drawCard(), myDeck.drawCard()};
    std::vector<Card> tableCards;

    std::string phases[] = {"FLOP", "TURN", "RIVER"};
    for (int i = 0; i < 3; i++) {
        refreshUI(playerHand, tableCards, phases[i]);
        std::cout << "\nPress ANY KEY to continue...";
        _getch();
        if (i == 0) { 
            tableCards.push_back(myDeck.drawCard());
            tableCards.push_back(myDeck.drawCard());
            tableCards.push_back(myDeck.drawCard());
        } else {
            tableCards.push_back(myDeck.drawCard());
        }
    }

    refreshUI(playerHand, tableCards, "FINAL RIVER");
    std::cout << "\nPress ANY KEY for SHOWDOWN...";
    _getch();

    HandResult playerResult = evaluateHand(playerHand, tableCards);
    HandResult cpuResult = evaluateHand(cpuHand, tableCards);

    clearScreen();
    printHeader();
    std::cout << RED << BOLD << "\n*** SHOWDOWN ***" << RESET << "\n\n";
    std::cout << "YOUR HAND: "; for (const auto& c : playerHand) c.print();
    std::cout << " (" << playerResult.name << ")\n";
    std::cout << "CPU HAND:  "; for (const auto& c : cpuHand) c.print();
    std::cout << " (" << cpuResult.name << ")\n\n";

    // PAYOUT LOGIC
    if (playerResult.totalScore > cpuResult.totalScore) {
        int winnings = currentBet * static_cast<int>(playerResult.value);
        playerBalance += winnings;
        std::cout << GREEN << BOLD << ">> YOU WIN! <<" << RESET << "\n";
        std::cout << "Hand Multiplier: " << static_cast<int>(playerResult.value) << "x\n";
        std::cout << "Profit: $" << winnings << std::endl;
    } else if (cpuResult.totalScore > playerResult.totalScore) {
        playerBalance -= currentBet;
        std::cout << RED << BOLD << ">> CPU WINS! <<" << RESET << "\n";
        std::cout << "Lost Bet: -$" << currentBet << std::endl;
    } else {
        std::cout << YELLOW << BOLD << ">> DRAW! Bet returned. <<" << RESET << std::endl;
    }

    std::cout << BOLD << "\nNEW BALANCE: $" << playerBalance << RESET << std::endl;
    waitForEnter();
}


int main() {
    bootingSequence();
    
    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        return -1;
    }

    ma_engine_play_sound(&engine, "sound.wav", NULL);

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
            if (choice == 0) playGame();
            else if (choice == 1) showFile(".files\\rules.txt");
            else if (choice == 2) showFile(".files\\points.txt");
            else if (choice == 3) break;
        }
    }

    ma_engine_uninit(&engine);
    return 0;
}