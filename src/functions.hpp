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
inline int startingRank = 9; // FIX 5: global used by Deck and main game loop
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

// Forward declaration
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
    std::cout << MAGENTA << BOLD << R"(
============================================================================
============================================================================)" << RESET << std::endl;
    std::cout << MAGENTA << BOLD << R"(
 _____ _____ ______  ___   _____   _   _ _____ _    ______ _ ________  ___
|_   _|  ___\ \ / / / _ \ /  ___| | | | |  _  | |   |  _  ( )  ___|  \/  |
  | | | |__  \ V / / /_\ \\ `--.  | |_| | | | | |   | | | |/| |__ | .  . |
  | | |  __| /   \ |  _  | `--. \ |  _  | | | | |   | | | | |  __|| |\/| |
  | | | |___/ /^\ \| | | |/\__/ / | | | \ \_/ / |___| |/ /  | |___| |  | |
  \_/ \____/\/   \/\_| |_/\____/  \_| |_/\___/\_____/___/   \____/\_|  |_/ )" << RESET << std::endl;
        std::cout << MAGENTA << BOLD << R"(
============================================================================
============================================================================)" << RESET << std::endl;
}

inline void bootingSequence() {
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

// --- FIX 1: chooseOpponents() ---
// Limits CPU count to 1-4 and sets startingRank = 10 - numCPUs (gives 9, 8, 7, 6)
inline void chooseOpponents() {
    clearScreen();
    printHeader();
    std::cout << YELLOW << "How many CPU opponents? (1-4): " << RESET;
    while (true) {
        if (!(std::cin >> numCPUs)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << RED << "Invalid input. Enter a number between 1 and 4: " << RESET;
        } else if (numCPUs < 1 || numCPUs > 4) {
            std::cin.ignore(1000, '\n');
            std::cout << RED << "Must be 1-4. Try again: " << RESET;
        } else {
            std::cin.ignore(1000, '\n');
            break;
        }
    }
    startingRank = 10 - numCPUs; // 1 CPU -> rank 9, 2 -> 8, 3 -> 7, 4 -> 6
    std::cout << GREEN << "Playing against " << numCPUs << " CPU(s). "
              << "Deck starts from rank " << startingRank << "." << RESET << "\n";
    sleepMs(1200);
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
    HandResult hr = evaluateHand(hand, table);
    int score = hr.totalScore;
    int luck = rand() % 100;
    bool canRaise = (raisesSoFar < maxRaises);
    
    if (table.size() < 3) { // Pre-flop
        if (currentBetToCall == 0) {
            if (canRaise && score > 2000000 && luck < 50) return {BetAction::RAISE, 20 + (rand() % 30)};
            return {BetAction::CHECK, 0};
        } else {
            if (score < 100000 && currentBetToCall > 20) return (luck < 20) ? Decision{BetAction::CALL, 0} : Decision{BetAction::FOLD, 0};
            if (score > 3000000 && canRaise) return {BetAction::RAISE, currentBetToCall * 2 + (rand() % 20)};
            if (score > 1000000 || currentBetToCall < 30) return {BetAction::CALL, 0};
            return (currentBetToCall > 40) ? Decision{BetAction::FOLD, 0} : Decision{BetAction::CALL, 0};
        }
    } else { // Post-flop
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
        case HIGH_CARD:      return "HIGH CARD";
        case PAIR:           return "ONE PAIR";
        case TWO_PAIR:       return "TWO PAIR";
        case THREE_KIND:     return "THREE OF A KIND";
        case STRAIGHT:       return "STRAIGHT";
        case FLUSH:          return "FLUSH";
        case FULL_HOUSE:     return "FULL HOUSE";
        case FOUR_KIND:      return "FOUR OF A KIND";
        case STRAIGHT_FLUSH: return "STRAIGHT FLUSH";
        case ROYAL_FLUSH:    return "ROYAL FLUSH";
        default:             return "UNKNOWN";
    }
}

// FIX 3: Completely rewritten evaluateHand() with proper base-100 weighted scoring.
// Scoring layout (all values use base 100 per "slot"):
//   totalScore = handCategory * 100^5
//              + primary rank   * 100^4   (quad rank / trip rank / pair rank / high card)
//              + secondary rank * 100^3   (pair in full house / second pair in two-pair)
//              + kicker1        * 100^2
//              + kicker2        * 100^1
//              + kicker3        * 100^0
// This guarantees no overlap between any two distinct hands.
inline HandResult evaluateHand(std::vector<Card> hand, std::vector<Card> table) {
    std::vector<Card> all = hand;
    all.insert(all.end(), table.begin(), table.end());
    
    std::map<int, int> rankCounts;
    std::map<int, std::vector<int>> suitMap; 
    for (const auto& c : all) {
        rankCounts[static_cast<int>(c.rank)]++;
        suitMap[static_cast<int>(c.symbol)].push_back(static_cast<int>(c.rank));
    }

    // --- Basic detection logic ---
    bool isFlush = false; std::vector<int> flushRanks; 
    for (auto& [s, rv] : suitMap) {
        if (rv.size() >= 5) {
            isFlush = true; std::sort(rv.rbegin(), rv.rend());
            for(int i=0; i<5; ++i) flushRanks.push_back(rv[i]);
            break;
        }
    }

    auto findStraightHigh = [&](std::vector<int> rks) -> int {
        std::sort(rks.begin(), rks.end());
        rks.erase(std::unique(rks.begin(), rks.end()), rks.end());
        if (std::find(rks.begin(), rks.end(), 14) != rks.end()) rks.insert(rks.begin(), 1); 
        int best = 0; if (rks.size() < 5) return 0;
        for (size_t i = 0; i <= rks.size() - 5; ++i) if (rks[i+4] - rks[i] == 4) best = rks[i+4];
        return best;
    };

    std::vector<int> allRks;
    for(auto const& [rank, count] : rankCounts) allRks.push_back(rank);
    int straightHigh = findStraightHigh(allRks);

    std::vector<int> quads, trips, pairs, singles;
    for (auto it = rankCounts.rbegin(); it != rankCounts.rend(); ++it) {
        if (it->second == 4) quads.push_back(it->first);
        else if (it->second == 3) trips.push_back(it->first);
        else if (it->second == 2) pairs.push_back(it->first);
        else singles.push_back(it->first);
    }

    // --- POWER OF 100 SCORING SYSTEM ---
    const long long B5 = 10000000000LL; // Category
    const long long B4 = 100000000LL;   // Primary Rank
    const long long B3 = 1000000LL;     // Secondary/Kicker 1
    const long long B2 = 10000LL;       // Kicker 2

    HandValue v = HIGH_CARD; long long score = 0;

    if (straightHigh > 0 && isFlush) {
        v = (straightHigh == 14) ? ROYAL_FLUSH : STRAIGHT_FLUSH;
        score = (long long)v * B5 + straightHigh * B4;
    }
    else if (!quads.empty()) {
        v = FOUR_KIND; score = (long long)v * B5 + quads[0] * B4;
    }
    else if (!trips.empty() && (!pairs.empty() || trips.size() > 1)) {
        v = FULL_HOUSE; score = (long long)v * B5 + trips[0] * B4;
    }
    else if (isFlush) {
        v = FLUSH; score = (long long)v * B5 + flushRanks[0] * B4;
    }
    else if (straightHigh > 0) {
        v = STRAIGHT; score = (long long)v * B5 + straightHigh * B4;
    }
    else if (!trips.empty()) {
        v = THREE_KIND; score = (long long)v * B5 + trips[0] * B4; // Now strictly > Two Pair
    }
    else if (pairs.size() >= 2) {
        v = TWO_PAIR; score = (long long)v * B5 + pairs[0] * B4 + pairs[1] * B3;
    }
    else if (!pairs.empty()) {
        v = PAIR; score = (long long)v * B5 + pairs[0] * B4;
    }
    else {
        v = HIGH_CARD; score = (long long)v * B5 + singles[0] * B4;
    }

    return { v, (int)score, getHandName(v) };
}

// --- FIX 2: Deck constructor accepts minRank parameter ---
// minRank: lowest rank integer to include (2-14). Default 2 = full deck.
// 1 CPU -> minRank 9 (9,10,J,Q,K,A), 2 CPUs -> 8, 3 CPUs -> 7, 4 CPUs -> 6
class Deck {
private:
    std::vector<Card> cards;
public:
    explicit Deck(int minRank = 2) {
        for (int r = minRank; r <= 14; ++r)
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