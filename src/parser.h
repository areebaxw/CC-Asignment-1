#pragma once
#include "grammar.h"
#include "first_follow.h"
#include "tree.h"
#include "error_handler.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

using namespace std;

// Structure to record each parsing step for trace visualization
struct ParseStep {
    int step;
    string stack;
    string input;
    string action;
};

class Parser {
public:
    // The LL(1) parsing table:
    //   table[NonTerminal][terminal] = production RHS (list of symbols)
    // We use a nested map for simplicity.
    map<string,
             map<string, vector<string>>> table;

    bool isLL1 = true;  // set to false if any cell has a conflict

    // Stores the trace from most recent parse
    vector<ParseStep> parseTrace;

    // Build the parsing table from the grammar and its FIRST/FOLLOW sets
    void buildTable(const Grammar& g, const FirstFollow& ff);

    // Display the table
    void printTable(const Grammar& g) const;

    // Parse a single token stream; returns the parse-tree root (nullptr on error)
    shared_ptr<TreeNode> parse(const vector<string>& tokens,
                                    const Grammar& g,
                                    const FirstFollow& ff,
                                    ErrorHandler& err);

    // Display the table as DOT graph (Graphviz format)
    void displayTableDOT(const Grammar& g) const;

    // Display the parsing trace as DOT table (Graphviz format)
    void displayParseLtraceDOT() const;
};
