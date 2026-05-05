#ifndef LR1PARSER_H
#define LR1PARSER_H

#include "Grammar.h"
#include "Items.h"
#include "ParsingTable.h"
#include "Stack.h"
#include "Tree.h"
#include <vector>

using namespace std;

// LR(1) Parser
class LR1Parser {
private:
    Grammar grammar;
    vector<LR1ItemSet> canonicalCollection;
    ParsingTable parsingTable;
    string outputDirectory;
    bool hasConflicts;
    
    // Helper functions
    LR1ItemSet closure(const LR1ItemSet& itemSet) const;
    LR1ItemSet gotoFunc(const LR1ItemSet& itemSet, const Symbol& symbol) const;
    
    void buildCanonicalCollection();
    void buildParsingTable();
    
public:
    LR1Parser(const Grammar& g);
    
    // Build the parser (construct LR(1) item sets and parsing table)
    bool build();
    
    // Parse a string
    bool parse(const vector<Symbol>& input, ParseTree*& resultTree);
    
    // Display results
    void displayCanonicalCollection() const;
    void displayParsingTable() const;
    void saveCanonicalCollectionToFile(const string& filename) const;
    void saveParsingTableToFile(const string& filename) const;
    
    // Getters
    int getNumberOfStates() const { return canonicalCollection.size(); }
    const vector<LR1ItemSet>& getCanonicalCollection() const { return canonicalCollection; }
    const ParsingTable& getParsingTable() const { return parsingTable; }
    void setOutputDirectory(const string& dir) { outputDirectory = dir; }
    bool hasConflictsDetected() const { return hasConflicts; }
    
    // Visualization methods
    void saveStateDiagramAsImage(const string& filename) const;
    void saveParsingTableAsImage(const string& filename) const;
    void saveStackVisualization(const vector<string>& stackStates,
                                const vector<string>& remainingInputs,
                                const vector<string>& actions,
                                const string& filename) const;
};

#endif
