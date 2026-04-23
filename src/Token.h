#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>

using namespace std;

// Symbol types: Terminal or NonTerminal
enum SymbolType {
    TERMINAL,
    NON_TERMINAL,
    EPSILON
};

// Represents a grammar symbol (terminal or non-terminal)
class Symbol {
public:
    string name;
    SymbolType type;
    
    Symbol();
    Symbol(string n, SymbolType t);
    
    bool operator==(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;
    bool operator<(const Symbol& other) const;
    
    string toString() const;
};

// Represents a production rule: NonTerminal -> symbol1 symbol2 ... symbolN
class Production {
public:
    Symbol leftSide;
    vector<Symbol> rightSide;
    int productionId;
    
    Production();
    Production(Symbol left, vector<Symbol> right);
    
    bool isEmpty() const;
    string toString() const;
    
    bool operator==(const Production& other) const;
    bool operator<(const Production& other) const;
};

#endif
