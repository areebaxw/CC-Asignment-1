#ifndef PARSINGTABLE_H
#define PARSINGTABLE_H

#include "Token.h"
#include "Items.h"
#include "Grammar.h"
#include <map>
#include <set>

using namespace std;

// Action type in parsing table
enum ActionType {
    SHIFT,
    REDUCE,
    ACCEPT,
    ERROR,
    NONE
};

// Parsing table action
class Action {
public:
    ActionType type;
    int value;  // For shift: next state; For reduce: production id
    
    Action();
    Action(ActionType t, int v);
    
    bool operator==(const Action& other) const;
    
    string toString() const;
};

// Parsing table (used by both SLR(1) and LR(1))
class ParsingTable {
protected:
    map<pair<int, Symbol>, Action> actionTable;
    map<pair<int, Symbol>, int> gotoTable;
    
    int numStates;
    
public:
    ParsingTable();
    void clear();
    
    void setAction(int state, const Symbol& symbol, const Action& action);
    void setGoto(int state, const Symbol& symbol, int nextState);
    
    Action getAction(int state, const Symbol& symbol) const;
    int getGoto(int state, const Symbol& symbol) const;
    
    void displayTable(const set<Symbol>& terminals, const set<Symbol>& nonTerminals) const;
    void saveTableToFile(const string& filename, const set<Symbol>& terminals, 
                         const set<Symbol>& nonTerminals) const;
    
    void setNumStates(int n) { numStates = n; }
    int getNumStates() const { return numStates; }
    
    // Check for conflicts
    map<pair<int, Symbol>, pair<Action, Action>> findShiftReduceConflicts() const;
    vector<pair<pair<int, Symbol>, vector<Action>>> findReduceReduceConflicts() const;
};

#endif
