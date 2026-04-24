#include "Items.h"

// LR0Item Implementation
LR0Item::LR0Item() : dotPosition(0) {}

LR0Item::LR0Item(const Production& prod, int dot) 
    : production(prod), dotPosition(dot) {}

bool LR0Item::operator==(const LR0Item& other) const {
    return production == other.production && dotPosition == other.dotPosition;
}

bool LR0Item::operator<(const LR0Item& other) const {
    if (!(production == other.production)) return production < other.production;
    return dotPosition < other.dotPosition;
}

Symbol LR0Item::getSymbolAfterDot() const {
    if (dotPosition < production.rightSide.size()) {
        return production.rightSide[dotPosition];
    }
    return Symbol("", TERMINAL);
}

bool LR0Item::isDotAtEnd() const {
    return dotPosition >= production.rightSide.size();
}

string LR0Item::toString() const {
    string result = production.leftSide.toString() + " -> ";
    
    for (int i = 0; i < production.rightSide.size(); i++) {
        if (i == dotPosition) result += ". ";
        result += production.rightSide[i].toString();
        if (i < production.rightSide.size() - 1 && i != dotPosition - 1) result += " ";
    }
    
    if (dotPosition >= production.rightSide.size()) {
        result += " .";
    }
    
    return result;
}

// LR1Item Implementation
LR1Item::LR1Item() : dotPosition(0), lookahead("", TERMINAL) {}

LR1Item::LR1Item(const Production& prod, int dot, const Symbol& la) 
    : production(prod), dotPosition(dot), lookahead(la) {}

bool LR1Item::operator==(const LR1Item& other) const {
    return production == other.production && 
           dotPosition == other.dotPosition && 
           lookahead == other.lookahead;
}

bool LR1Item::operator<(const LR1Item& other) const {
    if (!(production == other.production)) return production < other.production;
    if (dotPosition != other.dotPosition) return dotPosition < other.dotPosition;
    return lookahead < other.lookahead;
}

Symbol LR1Item::getSymbolAfterDot() const {
    if (dotPosition < production.rightSide.size()) {
        return production.rightSide[dotPosition];
    }
    return Symbol("", TERMINAL);
}

bool LR1Item::isDotAtEnd() const {
    return dotPosition >= production.rightSide.size();
}

string LR1Item::toString() const {
    string result = "[" + production.leftSide.toString() + " -> ";
    
    for (int i = 0; i < production.rightSide.size(); i++) {
        if (i == dotPosition) result += ". ";
        result += production.rightSide[i].toString();
        if (i < production.rightSide.size() - 1 && i != dotPosition - 1) result += " ";
    }
    
    if (dotPosition >= production.rightSide.size()) {
        result += " .";
    }
    
    result += ", " + lookahead.toString() + "]";
    
    return result;
}

// LR0ItemSet Implementation
LR0ItemSet::LR0ItemSet() : stateNumber(-1) {}

bool LR0ItemSet::operator==(const LR0ItemSet& other) const {
    return items == other.items;
}

bool LR0ItemSet::operator<(const LR0ItemSet& other) const {
    return items < other.items;
}

bool LR0ItemSet::contains(const LR0Item& item) const {
    return items.find(item) != items.end();
}

void LR0ItemSet::addItem(const LR0Item& item) {
    items.insert(item);
}

string LR0ItemSet::toString() const {
    string result = "";
    for (const LR0Item& item : items) {
        result += item.toString() + "\n";
    }
    return result;
}

// LR1ItemSet Implementation
LR1ItemSet::LR1ItemSet() : stateNumber(-1) {}

bool LR1ItemSet::operator==(const LR1ItemSet& other) const {
    return items == other.items;
}

bool LR1ItemSet::operator<(const LR1ItemSet& other) const {
    return items < other.items;
}

bool LR1ItemSet::contains(const LR1Item& item) const {
    return items.find(item) != items.end();
}

void LR1ItemSet::addItem(const LR1Item& item) {
    items.insert(item);
}

string LR1ItemSet::toString() const {
    string result = "";
    for (const LR1Item& item : items) {
        result += item.toString() + "\n";
    }
    return result;
}
