#include "Token.h"

// Symbol Implementation
Symbol::Symbol() : name(""), type(TERMINAL) {}

Symbol::Symbol(string n, SymbolType t) : name(n), type(t) {}

bool Symbol::operator==(const Symbol& other) const {
    return name == other.name && type == other.type;
}

bool Symbol::operator!=(const Symbol& other) const {
    return !(*this == other);
}

bool Symbol::operator<(const Symbol& other) const {
    if (name != other.name) return name < other.name;
    return type < other.type;
}

string Symbol::toString() const {
    if (type == EPSILON) return "ε";
    return name;
}

// Production Implementation
Production::Production() : productionId(-1) {}

Production::Production(Symbol left, vector<Symbol> right) 
    : leftSide(left), rightSide(right), productionId(-1) {}

bool Production::isEmpty() const {
    return rightSide.empty() || (rightSide.size() == 1 && rightSide[0].type == EPSILON);
}

string Production::toString() const {
    string result = leftSide.toString() + " -> ";
    
    if (rightSide.empty()) {
        result += "ε";
    } else {
        for (int i = 0; i < rightSide.size(); i++) {
            if (i > 0) result += " ";
            result += rightSide[i].toString();
        }
    }
    return result;
}

bool Production::operator==(const Production& other) const {
    if (!(leftSide == other.leftSide)) return false;
    if (rightSide.size() != other.rightSide.size()) return false;
    
    for (int i = 0; i < rightSide.size(); i++) {
        if (!(rightSide[i] == other.rightSide[i])) return false;
    }
    return true;
}

bool Production::operator<(const Production& other) const {
    if (!(leftSide == other.leftSide)) return leftSide < other.leftSide;
    if (rightSide.size() != other.rightSide.size()) return rightSide.size() < other.rightSide.size();
    
    for (int i = 0; i < rightSide.size(); i++) {
        if (rightSide[i] != other.rightSide[i]) return rightSide[i] < other.rightSide[i];
    }
    return false;
}
