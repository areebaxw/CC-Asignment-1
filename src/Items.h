#ifndef ITEMS_H
#define ITEMS_H

#include "Token.h"
#include <set>
#include <map>

using namespace std;

// LR(0) Item: A -> alpha • beta
class LR0Item {
public:
    Production production;
    int dotPosition;  // Position of dot in rightSide
    
    LR0Item();
    LR0Item(const Production& prod, int dot);
    
    bool operator==(const LR0Item& other) const;
    bool operator<(const LR0Item& other) const;
    
    // Get symbol after dot (if exists)
    Symbol getSymbolAfterDot() const;
    
    // Check if dot is at end
    bool isDotAtEnd() const;
    
    string toString() const;
};

// LR(1) Item: [A -> alpha • beta, lookahead]
class LR1Item {
public:
    Production production;
    int dotPosition;
    Symbol lookahead;
    
    LR1Item();
    LR1Item(const Production& prod, int dot, const Symbol& la);
    
    bool operator==(const LR1Item& other) const;
    bool operator<(const LR1Item& other) const;
    
    Symbol getSymbolAfterDot() const;
    bool isDotAtEnd() const;
    
    string toString() const;
};

// Set of LR(0) items representing a state
class LR0ItemSet {
public:
    set<LR0Item> items;
    int stateNumber;
    
    LR0ItemSet();
    
    bool operator==(const LR0ItemSet& other) const;
    bool operator<(const LR0ItemSet& other) const;
    
    bool contains(const LR0Item& item) const;
    void addItem(const LR0Item& item);
    
    string toString() const;
};

// Set of LR(1) items representing a state
class LR1ItemSet {
public:
    set<LR1Item> items;
    int stateNumber;
    
    LR1ItemSet();
    
    bool operator==(const LR1ItemSet& other) const;
    bool operator<(const LR1ItemSet& other) const;
    
    bool contains(const LR1Item& item) const;
    void addItem(const LR1Item& item);
    
    string toString() const;
};

#endif
