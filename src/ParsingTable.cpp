#include "ParsingTable.h"
#include <iostream>
#include <fstream>
#include <iomanip>

// Action Implementation
Action::Action() : type(NONE), value(-1) {}

Action::Action(ActionType t, int v) : type(t), value(v) {}

bool Action::operator==(const Action& other) const {
    return type == other.type && value == other.value;
}

string Action::toString() const {
    switch (type) {
        case SHIFT:
            return "s" + to_string(value);
        case REDUCE:
            return "r" + to_string(value);
        case ACCEPT:
            return "accept";
        case ERROR:
            return "error";
        case NONE:
            return "";
        default:
            return "?";
    }
}

// ParsingTable Implementation
ParsingTable::ParsingTable() : numStates(0) {}

void ParsingTable::clear() {
    actionTable.clear();
    gotoTable.clear();
    numStates = 0;
}

void ParsingTable::setAction(int state, const Symbol& symbol, const Action& action) {
    pair<int, Symbol> key(state, symbol);
    actionTable[key] = action;
}

void ParsingTable::setGoto(int state, const Symbol& symbol, int nextState) {
    pair<int, Symbol> key(state, symbol);
    gotoTable[key] = nextState;
}

Action ParsingTable::getAction(int state, const Symbol& symbol) const {
    pair<int, Symbol> key(state, symbol);
    auto it = actionTable.find(key);
    if (it != actionTable.end()) {
        return it->second;
    }
    return Action(ERROR, -1);
}

int ParsingTable::getGoto(int state, const Symbol& symbol) const {
    pair<int, Symbol> key(state, symbol);
    auto it = gotoTable.find(key);
    if (it != gotoTable.end()) {
        return it->second;
    }
    return -1;
}

void ParsingTable::displayTable(const set<Symbol>& terminals, const set<Symbol>& nonTerminals) const {
    cout << "\n=== Parsing Table ===" << endl;
    
    // Header: states and symbols
    cout << "State\t";
    
    // Print terminal columns (ACTION)
    for (const Symbol& term : terminals) {
        cout << term.toString() << "\t";
    }
    
    // Print non-terminal columns (GOTO)
    for (const Symbol& nonTerm : nonTerminals) {
        if (nonTerm.name.find("Prime") == string::npos) {  // Skip augmented start
            cout << nonTerm.toString() << "\t";
        }
    }
    cout << endl;
    
    // Print rows for each state
    for (int i = 0; i < numStates; i++) {
        cout << "I" << i << "\t";
        
        // ACTION entries
        for (const Symbol& term : terminals) {
            Action act = getAction(i, term);
            if (act.type != NONE) {
                cout << act.toString();
            }
            cout << "\t";
        }
        
        // GOTO entries
        for (const Symbol& nonTerm : nonTerminals) {
            if (nonTerm.name.find("Prime") == string::npos) {
                int go = getGoto(i, nonTerm);
                if (go >= 0) {
                    cout << go;
                }
                cout << "\t";
            }
        }
        cout << endl;
    }
}

void ParsingTable::saveTableToFile(const string& filename, const set<Symbol>& terminals,
                                   const set<Symbol>& nonTerminals) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot create file " << filename << endl;
        return;
    }
    
    // Header
    file << "State\t";
    for (const Symbol& term : terminals) {
        file << term.toString() << "\t";
    }
    for (const Symbol& nonTerm : nonTerminals) {
        if (nonTerm.name.find("Prime") == string::npos) {
            file << nonTerm.toString() << "\t";
        }
    }
    file << endl;
    
    // Rows
    for (int i = 0; i < numStates; i++) {
        file << "I" << i << "\t";
        
        for (const Symbol& term : terminals) {
            Action act = getAction(i, term);
            if (act.type != NONE) {
                file << act.toString();
            }
            file << "\t";
        }
        
        for (const Symbol& nonTerm : nonTerminals) {
            if (nonTerm.name.find("Prime") == string::npos) {
                int go = getGoto(i, nonTerm);
                if (go >= 0) {
                    file << go;
                }
                file << "\t";
            }
        }
        file << endl;
    }
    
    file.close();
}

map<pair<int, Symbol>, pair<Action, Action>> ParsingTable::findShiftReduceConflicts() const {
    map<pair<int, Symbol>, pair<Action, Action>> conflicts;
    
    // For each cell in ACTION table
    for (const auto& entry : actionTable) {
        int state = entry.first.first;
        const Symbol& symbol = entry.first.second;
        const Action& action1 = entry.second;
        
        // Check if there's another action for the same state-symbol pair
        // This is a simplified check; in real implementation, we'd track multiple actions
    }
    
    return conflicts;
}

vector<pair<pair<int, Symbol>, vector<Action>>> ParsingTable::findReduceReduceConflicts() const {
    vector<pair<pair<int, Symbol>, vector<Action>>> conflicts;
    
    // For each cell in ACTION table, count reduce actions
    map<pair<int, Symbol>, vector<Action>> reduceActions;
    
    for (const auto& entry : actionTable) {
        const Action& action = entry.second;
        if (action.type == REDUCE) {
            reduceActions[entry.first].push_back(action);
        }
    }
    
    // Find cells with multiple reduce actions
    for (const auto& entry : reduceActions) {
        if (entry.second.size() > 1) {
            conflicts.push_back(entry);
        }
    }
    
    return conflicts;
}
