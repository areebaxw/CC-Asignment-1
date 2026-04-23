#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "Token.h"
#include <set>
#include <map>
#include <iostream>
#include <fstream>

using namespace std;

// Represents a Context-Free Grammar
class Grammar {
private:
    vector<Production> productions;
    Symbol startSymbol;
    Symbol augmentedStartSymbol;
    set<Symbol> terminals;
    set<Symbol> nonTerminals;
    map<Symbol, set<Symbol>> firstSets;
    map<Symbol, set<Symbol>> followSets;
    
    bool isTerminal(const string& symbol) const;
    bool isNonTerminal(const string& symbol) const;
    void computeFirstSets();
    void computeFollowSets();
    
public:
    Grammar();
    
    // Load grammar from file
    bool loadFromFile(const string& filename);
    
    // Augment grammar with new start symbol
    void augmentGrammar();
    
    // Getters
    const vector<Production>& getProductions() const { return productions; }
    Symbol getStartSymbol() const { return startSymbol; }
    Symbol getAugmentedStartSymbol() const { return augmentedStartSymbol; }
    const set<Symbol>& getTerminals() const { return terminals; }
    const set<Symbol>& getNonTerminals() const { return nonTerminals; }
    
    // FIRST and FOLLOW sets
    set<Symbol> getFirstSet(const Symbol& symbol) const;
    set<Symbol> getFirstSetOfString(const vector<Symbol>& symbols) const;
    set<Symbol> getFollowSet(const Symbol& symbol) const;
    
    // Get productions for a specific non-terminal
    vector<Production> getProductionsFor(const Symbol& nonTerminal) const;
    
    // Display grammar
    void displayGrammar() const;
    void displayAugmentedGrammar() const;
    void displayFirstAndFollowSets() const;
    void saveAugmentedGrammarToFile(const string& filename) const;
    
    // Utility function to add end-of-input marker
    static const Symbol END_MARKER;
};

#endif
