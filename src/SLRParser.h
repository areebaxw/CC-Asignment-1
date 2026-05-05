#ifndef SLRPARSER_H
#define SLRPARSER_H

#include "Grammar.h"
#include "Items.h"
#include "ParsingTable.h"
#include "Stack.h"
#include "Tree.h"
#include <vector>

using namespace std;

// SLR(1) Parser
class SLRParser {
private:
    Grammar grammar;
    vector<LR0ItemSet> canonicalCollection;
    ParsingTable parsingTable;
    string outputDirectory;
    bool hasConflicts;
    
    // Helper functions
    LR0ItemSet closure(const LR0ItemSet& itemSet) const;
    LR0ItemSet gotoFunc(const LR0ItemSet& itemSet, const Symbol& symbol) const;
    
    void buildCanonicalCollection();
    void buildParsingTable();
    
public:
    SLRParser(const Grammar& g);
    
    // Build the parser (construct item sets and parsing table)
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
    const vector<LR0ItemSet>& getCanonicalCollection() const { return canonicalCollection; }
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
