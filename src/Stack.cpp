#include "Stack.h"

// StackEntry Implementation
StackEntry::StackEntry() : state(-1) {}

StackEntry::StackEntry(const Symbol& sym, int st) 
    : symbol(sym), state(st) {}

string StackEntry::toString() const {
    if (state >= 0) {
        return to_string(state);
    }
    return symbol.toString();
}

// ParsingStack Implementation
ParsingStack::ParsingStack() {}

void ParsingStack::pushSymbol(const Symbol& symbol) {
    StackEntry entry(symbol, -1);
    stack.push_back(entry);
}

void ParsingStack::pushState(int state) {
    StackEntry entry(Symbol("", TERMINAL), state);
    stack.push_back(entry);
}

Symbol ParsingStack::popSymbol() {
    if (!stack.empty()) {
        Symbol result = stack.back().symbol;
        stack.pop_back();
        return result;
    }
    return Symbol("", TERMINAL);
}

int ParsingStack::popState() {
    if (!stack.empty()) {
        int result = stack.back().state;
        stack.pop_back();
        return result;
    }
    return -1;
}

int ParsingStack::topState() const {
    for (int i = stack.size() - 1; i >= 0; i--) {
        if (stack[i].state >= 0) {
            return stack[i].state;
        }
    }
    return -1;
}

Symbol ParsingStack::topSymbol() const {
    for (int i = stack.size() - 1; i >= 0; i--) {
        if (stack[i].state < 0) {
            return stack[i].symbol;
        }
    }
    return Symbol("", TERMINAL);
}

void ParsingStack::clear() {
    stack.clear();
}

bool ParsingStack::isEmpty() const {
    return stack.empty();
}

int ParsingStack::size() const {
    return stack.size();
}

Symbol ParsingStack::getSymbolAt(int position) const {
    if (position >= 0 && position < stack.size()) {
        return stack[position].symbol;
    }
    return Symbol("", TERMINAL);
}

string ParsingStack::toString() const {
    string result = "[ ";
    for (int i = 0; i < stack.size(); i++) {
        result += stack[i].toString();
        if (i < stack.size() - 1) result += " ";
    }
    result += " ]";
    return result;
}
