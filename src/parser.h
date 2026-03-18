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

class Parser {
public:
    // The LL(1) parsing table:
    //   table[NonTerminal][terminal] = production RHS (list of symbols)
    // We use a nested map for simplicity.
    map<string,
             map<string, vector<string>>> table;

    bool isLL1 = true;  // set to false if any cell has a conflict

    // Build the parsing table from the grammar and its FIRST/FOLLOW sets
    void buildTable(const Grammar& g, const FirstFollow& ff);

    // Display the table
    void printTable(const Grammar& g) const;

    // Parse a single token stream; returns the parse-tree root (nullptr if error)
    shared_ptr<TreeNode> parse(const vector<string>& tokens,
                                    const Grammar& g,
                                    const FirstFollow& ff,
                                    ErrorHandler& err);
};
