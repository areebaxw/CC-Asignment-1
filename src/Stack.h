#ifndef STACK_H
#define STACK_H

#include "Token.h"
#include <vector>
#include <string>

using namespace std;

// Stack entry: either a symbol or a state number
class StackEntry {
public:
    Symbol symbol;
    int state;
    
    StackEntry();
    StackEntry(const Symbol& sym, int st);
    
    string toString() const;
};

// Parsing stack: stores alternating symbols and states
class ParsingStack {
private:
    vector<StackEntry> stack;
    
public:
    ParsingStack();
    
    void pushSymbol(const Symbol& symbol);
    void pushState(int state);
    
    Symbol popSymbol();
    int popState();
    
    int topState() const;
    Symbol topSymbol() const;
    
    void clear();
    bool isEmpty() const;
    int size() const;
    
    // Get symbol at position (from bottom)
    Symbol getSymbolAt(int position) const;
    
    // For display
    string toString() const;
};

#endif
