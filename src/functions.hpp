#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>
#include <thread>
#include <chrono>
#include <conio.h>
#include <map>
#include <set>
#include <cstdlib>
#include <ctime>

// ANSI Colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define CYAN    "\033[36m"
#define YELLOW  "\033[33m"
#define BOLD    "\033[1m"
#define GREEN   "\033[32m"
#define MAGENTA "\033[35m"

// --- GLOBAL ECONOMY ---
inline int playerBalance = 1000; 
inline int currentBet = 0;
inline int pot = 0;
inline int numCPUs = 1;
const int MAX_RAISES = 3; // Betting cap per round

enum class Rank { TWO = 2, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };
enum class Symbol { SPADES = 1, CLUBS = 2, DIAMONDS = 3, HEART = 4 };

enum HandValue { 
    HIGH_CARD = 1, PAIR = 2, TWO_PAIR = 3, THREE_KIND = 4, 
    STRAIGHT = 5, FLUSH = 6, FULL_HOUSE = 7, FOUR_KIND = 8, 
    STRAIGHT_FLUSH = 9, ROYAL_FLUSH = 10 
};

enum class BetAction { FOLD, CHECK, CALL, RAISE };

struct HandResult {
    HandValue value;
    int totalScore; 
    std::string name;
};

struct Decision {
    BetAction action;
    int amount;
};

struct Card {
    Rank rank;
    Symbol symbol;
    void print() const {
        std::string r[] = {"", "", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
        std::string s[] = {"", "S", "C", "D", "H"};
        if (symbol == Symbol::HEART || symbol == Symbol::DIAMONDS) std::cout << RED;
        else std::cout << CYAN;
        std::cout << "[" << r[static_cast<int>(rank)] << s[static_cast<int>(symbol)] << "]" << RESET << " ";
    }
};

// Forward declaration - FIX for "evaluateHand was not declared in this scope"
HandResult evaluateHand(std::vector<Card> hand, std::vector<Card> table);

// --- TIMING UTILS ---
inline void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void sleepRandom() {
    sleepMs(1000 + (rand() % 1000)); // 1-2 second delay
}

// --- VISUALS ---
inline void clearScreen() { system("cls"); }

inline void printHeader() {
    std::cout << MAGENTA << BOLD << "========================================" << RESET << std::endl;
    std::cout << MAGENTA << BOLD << "       TEXAS HOLD'EM POKER ENGINE       " << RESET << std::endl;
    std::cout << MAGENTA << BOLD << "========================================" << RESET << std::endl;
}

inline void bootingSequence() {
    // Seed random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    clearScreen();
    std::cout << CYAN << "[SYSTEM]: Initializing Audio & Cards..." << RESET << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

inline void waitForEnter() {
    std::cout << "\n" << YELLOW << "Press ENTER to return to menu..." << RESET;
    while (_getch() != 13);
}

inline void showFile(std::string filename) {
    clearScreen(); printHeader();
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) std::cout << line << std::endl;
        file.close();
    }
    waitForEnter();
}

// --- BETTING & CPU LOGIC ---
inline void placeBet() {
    if (playerBalance <= 0) {
        std::cout << RED << "\n[!] You're broke! The dealer grants you $200 charity.\n" << RESET;
        playerBalance = 200;
    }

    while (true) {
        std::cout << GREEN << BOLD << "\nCURRENT BALANCE: $" << playerBalance << RESET << std::endl;
        std::cout << "Enter your bet amount: $";
        if (!(std::cin >> currentBet)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << RED << "Invalid input. Enter a number." << RESET << std::endl;
        } else if (currentBet <= 0) {
            std::cout << RED << "Bet must be greater than 0." << RESET << std::endl;
        } else if (currentBet > playerBalance) {
            std::cout << RED << "Insufficient funds!" << RESET << std::endl;
        } else {
            break; 
        }
    }
    std::cin.ignore(1000, '\n'); 
}

inline Decision cpuDecideBet(const std::vector<Card>& hand, const std::vector<Card>& table, 
                           int currentBetToCall, int raisesSoFar, int maxRaises) {
    // Evaluate current hand strength - FIX: removed unnecessary const_cast since evaluateHand takes by value
    HandResult hr = evaluateHand(hand, table);
    int score = hr.totalScore;
    int luck = rand() % 100;
    bool canRaise = (raisesSoFar < maxRaises);
    
    // In Texas Hold'em, early position betting is tighter
    if (table.size() < 3) { // Pre-flop (no community cards yet or just starting)
        if (currentBetToCall == 0) {
            if (canRaise && score > 2000000 && luck < 50) return {BetAction::RAISE, 20 + (rand() % 30)};
            return {BetAction::CHECK, 0};
        } else {
            if (score < 100000 && currentBetToCall > 20) return (luck < 20) ? Decision{BetAction::CALL, 0} : Decision{BetAction::FOLD, 0};
            if (score > 3000000 && canRaise) return {BetAction::RAISE, currentBetToCall * 2 + (rand() % 20)};
            if (score > 1000000 || currentBetToCall < 30) return {BetAction::CALL, 0};
            return (currentBetToCall > 40) ? Decision{BetAction::FOLD, 0} : Decision{BetAction::CALL, 0};
        }
    } else { // Post-flop with community cards
        if (currentBetToCall == 0) {
            if (canRaise && score > 2000000 && luck < 60) return {BetAction::RAISE, 25 + (rand() % 35)};
            else if (canRaise && score > 1500000 && luck < 30) return {BetAction::RAISE, 15 + (rand() % 20)};
            return {BetAction::CHECK, 0};
        } else {
            if (score < 150000 && currentBetToCall > 30) return (luck < 15) ? Decision{BetAction::CALL, 0} : Decision{BetAction::FOLD, 0};
            if (score > 4000000 && canRaise) return {BetAction::RAISE, currentBetToCall * 2 + (rand() % 25)};
            if (score > 2000000 && canRaise && luck < 40) return {BetAction::RAISE, currentBetToCall + 30};
            if (score > 1000000 || currentBetToCall < 40) return {BetAction::CALL, 0};
            return (currentBetToCall > 60) ? Decision{BetAction::FOLD, 0} : Decision{BetAction::CALL, 0};
        }
    }
}

// --- HAND EVALUATION ---
inline std::string getHandName(HandValue v) {
    switch(v) {
        case HIGH_CARD: return "HIGH CARD";
        case PAIR:      return "ONE PAIR";
        case TWO_PAIR:  return "TWO PAIR";
        case THREE_KIND:return "THREE OF A KIND";
        case STRAIGHT:  return "STRAIGHT";
        case FLUSH:     return "FLUSH";
        case FULL_HOUSE:return "FULL HOUSE";
        case FOUR_KIND: return "FOUR OF A KIND";
        case STRAIGHT_FLUSH: return "STRAIGHT FLUSH";
        case ROYAL_FLUSH:    return "ROYAL FLUSH";
        default: return "UNKNOWN";
    }
}

inline HandResult evaluateHand(std::vector<Card> hand, std::vector<Card> table) {
    std::vector<Card> all = hand;
    all.insert(all.end(), table.begin(), table.end());
    std::sort(all.begin(), all.end(), [](Card a, Card b) { return a.rank < b.rank; });

    std::map<Rank, int> rankCounts;
    std::map<Symbol, std::vector<int>> suitMap;
    std::set<int> uniqueRanks;

    for (const auto& c : all) {
        rankCounts[c.rank]++;
        suitMap[c.symbol].push_back(static_cast<int>(c.rank));
        uniqueRanks.insert(static_cast<int>(c.rank));
    }

    bool isFlush = false;
    Symbol flushSuit = Symbol::SPADES;
    for (auto const& [suit, ranks] : suitMap) {
        if (ranks.size() >= 5) {
            isFlush = true;
            flushSuit = suit;
        }
    }

    bool isStraight = false;
    int straightHigh = 0;
    std::vector<int> rVec(uniqueRanks.begin(), uniqueRanks.end());
    if (rVec.size() >= 5) {
        for (size_t i = 0; i <= rVec.size() - 5; i++) {
            if (rVec[i+4] - rVec[i] == 4) { 
                isStraight = true; 
                straightHigh = rVec[i+4]; 
            }
        }
        // Check for A-2-3-4-5 straight (wheel)
        if (!isStraight && rVec.size() >= 5) {
            if (rVec[0] == 2 && rVec[1] == 3 && rVec[2] == 4 && rVec[3] == 5 && rVec.back() == 14) {
                isStraight = true;
                straightHigh = 5;
            }
        }
    }

    int quads = 0, trips = 0, pairs = 0;
    for (auto const& [rank, count] : rankCounts) {
        if (count == 4) quads++;
        else if (count == 3) trips++;
        else if (count == 2) pairs++;
    }

    HandValue v = HIGH_CARD;
    if (isFlush && isStraight && straightHigh == 14) v = ROYAL_FLUSH;
    else if (isFlush && isStraight) v = STRAIGHT_FLUSH;
    else if (quads > 0) v = FOUR_KIND;
    else if (trips > 0 && pairs > 0) v = FULL_HOUSE;
    else if (isFlush) v = FLUSH;
    else if (isStraight) v = STRAIGHT;
    else if (trips > 0) v = THREE_KIND;
    else if (pairs >= 2) v = TWO_PAIR;
    else if (pairs == 1) v = PAIR;

    return { v, (static_cast<int>(v) * 100) + straightHigh, getHandName(v) };
}

// --- DECK ---
class Deck {
private:
    std::vector<Card> cards;
public:
    Deck() {
        for (int r = 2; r <= 14; ++r)
            for (int s = 1; s <= 4; ++s)
                cards.push_back({static_cast<Rank>(r), static_cast<Symbol>(s)});
    }
    void shuffleDeck() {
        std::random_device rd; 
        std::mt19937 g(rd());
        std::shuffle(cards.begin(), cards.end(), g);
    }
    Card drawCard() { 
        if (cards.empty()) return {Rank::TWO, Symbol::SPADES};
        Card drawn = cards.back(); 
        cards.pop_back(); 
        return drawn; 
    }
    bool isEmpty() const { return cards.empty(); }
    size_t remaining() const { return cards.size(); }
};

#endif

